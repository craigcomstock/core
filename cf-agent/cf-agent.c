/*
  Copyright 2024 Northern.tech AS

  This file is part of CFEngine 3 - written and maintained by Northern.tech AS.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; version 3.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of CFEngine, the applicable Commercial Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/


#include <platform.h>
#include <generic_agent.h>

#include <actuator.h>
#include <audit.h>
#include <cleanup.h>
#include <eval_context.h>
#include <verify_classes.h>
#include <verify_databases.h>
#include <verify_environments.h>
#include <verify_exec.h>
#include <verify_methods.h>
#include <verify_processes.h>
#include <verify_packages.h>
#include <verify_users.h>
#include <verify_services.h>
#include <verify_storage.h>
#include <verify_files.h>
#include <verify_files_utils.h>
#include <verify_vars.h>
#include <addr_lib.h>
#include <files_names.h>
#include <files_interfaces.h>
#include <files_repository.h>
#include <files_edit.h>
#include <files_properties.h>
#include <item_lib.h>
#include <vars.h>
#include <conversion.h>
#include <expand.h>
#include <locks.h>
#include <scope.h>
#include <matching.h>
#include <match_scope.h>
#include <instrumentation.h>
#include <promises.h>
#include <unix.h>
#include <attributes.h>
#include <communication.h>
#include <signals.h>
#include <nfs.h>
#include <processes_select.h>
#include <list.h>
#include <fncall.h>
#include <rlist.h>
#include <agent-diagnostics.h>
#include <known_dirs.h>
#include <cf-agent-enterprise-stubs.h>
#include <syslog_client.h>
#include <bootstrap.h>
#include <policy_server.h>
#include <misc_lib.h>
#include <buffer.h>
#include <loading.h>
#include <conn_cache.h>                 /* ConnCache_Init,ConnCache_Destroy */
#include <net.h>
#include <package_module.h>
#include <string_lib.h>
#include <cfnet.h>
#include <sys/types.h>                  /* checking umask on writing setxid log */
#include <sys/stat.h>                   /* checking umask on writing setxid log */
#include <ip_address.h>

#include <syntax.h>                     /* IsBuiltInPromiseType() */
#include <mod_common.h>
#include <mod_custom.h>                 /* EvaluateCustomPromise(), Intialize/FinalizeCustomPromises() */

#include <ornaments.h>
#include <cf-agent.h>


extern int PR_KEPT;
extern int PR_REPAIRED;
extern int PR_NOTKEPT;

bool ALLCLASSESREPORT = false; /* GLOBAL_P */
bool ALWAYS_VALIDATE = false; /* GLOBAL_P */
static bool CFPARANOID = false; /* GLOBAL_P */

static const Rlist *ACCESSLIST = NULL; /* GLOBAL_P */

int CFA_BACKGROUND = 0; /* GLOBAL_X */
static int CFA_BACKGROUND_LIMIT = 1; /* GLOBAL_P */

static Item *PROCESSREFRESH = NULL; /* GLOBAL_P */

static const char *const AGENT_TYPESEQUENCE[] =
{
    "meta",
    "vars",
    "defaults",
    "classes",                  /* Maelstrom order 2 */
    "users",
    "files",
    "packages",
    "guest_environments",
    "methods",
    "processes",
    "services",
    "commands",
    "storage",
    "databases",
    "reports",
    NULL
};

/*******************************************************************/
/* Agent specific variables                                        */
/*******************************************************************/

static void CheckAgentAccess(const Rlist *list, const Policy *policy);
static void KeepControlPromises(EvalContext *ctx, const Policy *policy, GenericAgentConfig *config);
static PromiseResult KeepAgentPromise(EvalContext *ctx, const Promise *pp, void *param);
static void NewTypeContext(TypeSequence type);
static void DeleteTypeContext(EvalContext *ctx, TypeSequence type);
static PromiseResult ParallelFindAndVerifyFilesPromises(EvalContext *ctx, const Promise *pp);
static void KeepPromiseBundles(EvalContext *ctx, const Policy *policy, GenericAgentConfig *config);
static int NoteBundleCompliance(const Bundle *bundle, int save_pr_kept, int save_pr_repaired, int save_pr_notkept, struct timespec start);
static void BannerStatusEnd(PromiseResult status, const char *type, char *name);
static void BannerStatusBegin(const char *type, char *name);
static PromiseResult DefaultVarPromise(EvalContext *ctx, const Promise *pp);

/**
 @brief
 Wrapper around DefaultVarPromise to silence cast-function-type compiler warning in ScheduleAgentOperations
 */
static PromiseResult DefaultVarPromiseWrapper(EvalContext *ctx, const Promise *pp, void *param) {
    UNUSED(param);
    return DefaultVarPromise(ctx, pp);
}

/*******************************************************************/

void ThisAgentInit(void)
{
    char filename[CF_BUFSIZE];

#ifdef HAVE_SETSID
    setsid();
#endif

    CFA_MAXTHREADS = 30;
    EDITFILESIZE = 100000;

/*
  do not set signal(SIGCHLD,SIG_IGN) in agent near
  popen() - or else pclose will fail to return
  status which we need for setting returns
*/

    snprintf(filename, CF_BUFSIZE, "%s/cfagent.%s.log", GetLogDir(), VSYSNAME.nodename);
    ToLowerStrInplace(filename);
    MapName(filename);

    const mode_t current_umask = umask(0777);  // Gets and changes umask
    umask(current_umask); // Restores umask
    Log(LOG_LEVEL_DEBUG, "Current umask is %o", current_umask);
    FILE *fp = safe_fopen(filename, "a");
    if (fp != NULL)
    {
        fclose(fp);
    }

    InitializeCustomPromises();
}

/*******************************************************************/

void KeepPromises(EvalContext *ctx, const Policy *policy, GenericAgentConfig *config)
{
    KeepControlPromises(ctx, policy, config);
    /* Check if 'abortclasses' aborted evaluation or not. */
    if (EvalAborted(ctx))
    {
        return;
    }
    KeepPromiseBundles(ctx, policy, config);
}

/*******************************************************************/
/* Level 2                                                         */
/*******************************************************************/

static void KeepControlPromises(EvalContext *ctx, const Policy *policy, GenericAgentConfig *config)
{
    Seq *constraints = ControlBodyConstraints(policy, AGENT_TYPE_AGENT);
    if (constraints)
    {
        for (size_t i = 0; i < SeqLength(constraints); i++)
        {
            Constraint *cp = SeqAt(constraints, i);

            if (!IsDefinedClass(ctx, cp->classes))
            {
                continue;
            }

            VarRef *ref = VarRefParseFromScope(cp->lval, "control_agent");
            DataType value_type;
            const void *value = EvalContextVariableGetPlaintext(ctx, ref, &value_type);
            VarRefDestroy(ref);

            /* If var not found */
            if (value_type == CF_DATA_TYPE_NONE)
            {
                Log(LOG_LEVEL_ERR, "Unknown lval '%s' in agent control body", cp->lval);
                continue;
            }

            /* 'files_single_copy => { }' is a perfectly valid case. */
            if (StringEqual(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_FSINGLECOPY].lval))
            {
                assert(value_type == CF_DATA_TYPE_STRING_LIST);
                SINGLE_COPY_LIST = value;
                SINGLE_COPY_CACHE = StringSetNew();
                if (WouldLog(LOG_LEVEL_VERBOSE))
                {
                    char *rlist_str = RlistToString(SINGLE_COPY_LIST);
                    Log(LOG_LEVEL_VERBOSE, "Setting file single copy list to: %s", rlist_str);
                    free(rlist_str);
                }
                continue;
            }

            /* Empty list is not supported for the other constraints/attributes. */
            if (value == NULL)
            {
                Log(LOG_LEVEL_ERR,
                    "Empty list is not a valid value for '%s' attribute in agent control body",
                    cp->lval);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_MAXCONNECTIONS].lval) == 0)
            {
                CFA_MAXTHREADS = (int) IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting maxconnections to %d", CFA_MAXTHREADS);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_CHECKSUM_ALERT_TIME].lval) == 0)
            {
                CF_PERSISTENCE = (int) IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting checksum_alert_time to %d", CF_PERSISTENCE);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_AGENTFACILITY].lval) == 0)
            {
                SetFacility(value);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_AGENTACCESS].lval) == 0)
            {
                ACCESSLIST = value;
                CheckAgentAccess(ACCESSLIST, policy);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_COPYFROM_RESTRICT_KEYS].lval) == 0)
            {
                EvalContextSetRestrictKeys(ctx, value);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_REFRESH_PROCESSES].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "Setting refresh_processes when starting to...");
                for (const Rlist *rp = value; rp != NULL; rp = rp->next)
                {
                    Log(LOG_LEVEL_VERBOSE, "%s", RlistScalarValue(rp));
                    // TODO: why is this only done in verbose mode?
                    // original commit says 'optimization'.
                    if (LogGetGlobalLevel() >= LOG_LEVEL_VERBOSE)
                    {
                        PrependItem(&PROCESSREFRESH, RlistScalarValue(rp), NULL);
                    }
                }
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_ABORTCLASSES].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "Setting abort classes from ...");

                for (const Rlist *rp = value; rp != NULL; rp = rp->next)
                {
                    char name[CF_MAXVARSIZE] = "";

                    strlcpy(name, RlistScalarValue(rp), CF_MAXVARSIZE);

                    EvalContextHeapAddAbort(ctx, name, cp->classes);
                }

                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_ABORTBUNDLECLASSES].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "Setting abort bundle classes from ...");

                for (const Rlist *rp = value; rp != NULL; rp = rp->next)
                {
                    char name[CF_MAXVARSIZE] = "";
                    strlcpy(name, RlistScalarValue(rp), CF_MAXVARSIZE);

                    EvalContextHeapAddAbortCurrentBundle(ctx, name, cp->classes);
                }

                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_ADDCLASSES].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "Add classes ...");

                for (const Rlist *rp = value; rp != NULL; rp = rp->next)
                {
                    Log(LOG_LEVEL_VERBOSE, "... %s", RlistScalarValue(rp));
                    EvalContextClassPutSoft(ctx, RlistScalarValue(rp), CONTEXT_SCOPE_NAMESPACE, "source=environment");
                }

                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_ALWAYSVALIDATE].lval) == 0)
            {
                ALWAYS_VALIDATE = BooleanFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting alwaysvalidate to '%s'", ALWAYS_VALIDATE ? "true" : "false");
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_ALLCLASSESREPORT].lval) == 0)
            {
                ALLCLASSESREPORT = BooleanFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting allclassesreport to '%s'", ALLCLASSESREPORT ? "true" : "false");
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_SECUREINPUT].lval) == 0)
            {
                CFPARANOID = BooleanFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting secure input to '%s'", CFPARANOID ? "true" : "false");
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_BINDTOINTERFACE].lval) == 0)
            {
                SetBindInterface(value);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_HASHUPDATES].lval) == 0)
            {
                bool enabled = BooleanFromString(value);

                SetChecksumUpdatesDefault(ctx, enabled);
                Log(LOG_LEVEL_VERBOSE, "Setting checksum updates to '%s'", enabled ? "true" : "false");
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_CHILDLIBPATH].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "Setting 'LD_LIBRARY_PATH=%s'", (const char *)value);
                setenv_wrapper("LD_LIBRARY_PATH", value, 1);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_DEFAULTCOPYTYPE].lval) == 0)
            {
                DEFAULT_COPYTYPE = value;
                Log(LOG_LEVEL_VERBOSE, "Setting defaultcopytype to '%s'", DEFAULT_COPYTYPE);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_FAUTODEFINE].lval) == 0)
            {
                SetFileAutoDefineList(value);
                Log(LOG_LEVEL_VERBOSE, "Setting file auto define list");
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_DRYRUN].lval) == 0)
            {
                EVAL_MODE = BooleanFromString(value) ? EVAL_MODE_DRY_RUN : EVAL_MODE_NORMAL;
                Log(LOG_LEVEL_VERBOSE, "Setting dryrun to %d", DONTDO);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_INFORM].lval) == 0)
            {
                bool inform = BooleanFromString(value);
                if (inform)
                {
                    LogSetGlobalLevel(MAX(LOG_LEVEL_INFO, LogGetGlobalLevel()));
                }
                else
                {
                    if (LogGetGlobalLevel() >= LOG_LEVEL_INFO)
                    {
                        LogSetGlobalLevel(LOG_LEVEL_NOTICE);
                    }
                }
                Log(LOG_LEVEL_VERBOSE, "body agent control, inform => '%s', sets new log level to '%s'",
                    inform ? "true" : "false", LogLevelToString(LogGetGlobalLevel()));
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_VERBOSE].lval) == 0)
            {
                bool verbose = BooleanFromString(value);
                if (verbose)
                {
                    LogSetGlobalLevel(MAX(LOG_LEVEL_VERBOSE, LogGetGlobalLevel()));
                }
                else
                {
                    if (LogGetGlobalLevel() >= LOG_LEVEL_VERBOSE)
                    {
                        LogSetGlobalLevel(LOG_LEVEL_INFO);
                    }
                }
                Log(LOG_LEVEL_VERBOSE, "body agent control, verbose => '%s', sets new log level to '%s'",
                    verbose ? "true" : "false", LogLevelToString(LogGetGlobalLevel()));
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_REPOSITORY].lval) == 0)
            {
                SetRepositoryLocation(value);
                Log(LOG_LEVEL_VERBOSE, "Setting repository to '%s'", (const char *)value);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_SKIPIDENTIFY].lval) == 0)
            {
                bool enabled = BooleanFromString(value);

                SetSkipIdentify(enabled);
                Log(LOG_LEVEL_VERBOSE, "Setting skipidentify to '%s'", enabled ? "true" : "false");
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_SUSPICIOUSNAMES].lval) == 0)
            {
                for (const Rlist *rp = value; rp != NULL; rp = rp->next)
                {
                    AddFilenameToListOfSuspicious(RlistScalarValue(rp));
                    Log(LOG_LEVEL_VERBOSE, "Considering '%s' as suspicious file", RlistScalarValue(rp));
                }

                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_REPCHAR].lval) == 0)
            {
                char c = *(char *)value;

                SetRepositoryChar(c);
                Log(LOG_LEVEL_VERBOSE, "Setting repchar to '%c'", c);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_MOUNTFILESYSTEMS].lval) == 0)
            {
                CF_MOUNTALL = BooleanFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting mountfilesystems to '%s'", CF_MOUNTALL ? "true" : "false");
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_EDITFILESIZE].lval) == 0)
            {
                EDITFILESIZE = IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting edit file size to %d", EDITFILESIZE);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_IFELAPSED].lval) == 0)
            {
                VIFELAPSED = IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting ifelapsed to %d", VIFELAPSED);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_EXPIREAFTER].lval) == 0)
            {
                VEXPIREAFTER = IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting expireafter to %d", VEXPIREAFTER);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_TIMEOUT].lval) == 0)
            {
                CONNTIMEOUT = IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting timeout to %jd", (intmax_t) CONNTIMEOUT);
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_MAX_CHILDREN].lval) == 0)
            {
                CFA_BACKGROUND_LIMIT = IntFromString(value);
                Log(LOG_LEVEL_VERBOSE, "Setting max_children to %d", CFA_BACKGROUND_LIMIT);
                if (CFA_BACKGROUND_LIMIT > 10)
                {
                    Log(LOG_LEVEL_ERR, "Silly value for max_children in agent control promise (%d > 10)",
                          CFA_BACKGROUND_LIMIT);
                    CFA_BACKGROUND_LIMIT = 1;
                }
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_ENVIRONMENT].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "Setting environment variables from ...");

                for (const Rlist *rp = value; rp != NULL; rp = rp->next)
                {
                    assert(strchr(RlistScalarValue(rp), '=')); /* Valid for putenv() */
                    if (putenv_wrapper(RlistScalarValue(rp)) != 0)
                    {
                        Log(LOG_LEVEL_ERR, "Failed to set environment variable '%s'. (putenv: %s)",
                            RlistScalarValue(rp), GetErrorStr());
                    }
                }

                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_SELECT_END_MATCH_EOF].lval) == 0)
            {
                Log(LOG_LEVEL_VERBOSE, "SET select_end_match_eof %s", (char *) value);
                EvalContextSetSelectEndMatchEof(ctx, BooleanFromString(value));
                continue;
            }

            if (strcmp(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_REPORTCLASSLOG].lval) == 0)
            {
                config->agent_specific.agent.report_class_log = BooleanFromString(value);

                Log(LOG_LEVEL_VERBOSE, "Setting report_class_log to %s",
                    config->agent_specific.agent.report_class_log? "true" : "false");
                continue;
            }

            if (StringEqual(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_EVALUATION_ORDER].lval))
            {
                assert(value_type == CF_DATA_TYPE_STRING);
                const char *evaluation_order = (char *) value;
                Log(LOG_LEVEL_VERBOSE, "SET evaluation %s", evaluation_order);

                if (StringEqual(evaluation_order, "top_down"))
                {
                    EvalContextSetAgentEvalOrder(ctx, EVAL_ORDER_TOP_DOWN);
                }
                else
                {
                    EvalContextSetAgentEvalOrder(ctx, EVAL_ORDER_CLASSIC);
                }
                continue;
            }

            if (StringEqual(cp->lval, CFA_CONTROLBODY[AGENT_CONTROL_DEFAULT_DIRECTORY_CREATE_MODE].lval))
            {
                assert(value_type == CF_DATA_TYPE_STRING);
                const char *mode_str = value;
                mode_t plus, minus;
                if (ParseModeString(mode_str, &plus, &minus))
                {
                    DEFAULTMODE |= plus;
                    DEFAULTMODE &= ~minus;
                    Log(LOG_LEVEL_VERBOSE, "Changed default directory create mode to %ju "
                        "(default_directory_create_mode => \"%s\")", (uintmax_t) DEFAULTMODE, mode_str);
                }
                else
                {
                    Log(LOG_LEVEL_ERR, "Failed to parse mode string for overriding default directory create mode "
                        "(default_directory_create_mode => \"%s\")", mode_str);
                }
                continue;
            }
        }
    }

    const void *value = NULL;
    if ((value = EvalContextVariableControlCommonGet(ctx, COMMON_CONTROL_LASTSEEN_EXPIRE_AFTER)))
    {
        LASTSEENEXPIREAFTER = IntFromString(value) * 60;
    }

    if ((value = EvalContextVariableControlCommonGet(ctx, COMMON_CONTROL_FIPS_MODE)))
    {
        FIPS_MODE = BooleanFromString(value);
        Log(LOG_LEVEL_VERBOSE, "Setting FIPS mode to '%s'", FIPS_MODE ? "true" : "false");
    }

    if ((value = EvalContextVariableControlCommonGet(ctx, COMMON_CONTROL_SYSLOG_PORT)))
    {
        SetSyslogPort(IntFromString(value));
        Log(LOG_LEVEL_VERBOSE, "Setting syslog_port to '%s'", (const char *)value);
    }

    if ((value = EvalContextVariableControlCommonGet(ctx, COMMON_CONTROL_SYSLOG_HOST)))
    {
        /* Don't resolve syslog_host now, better do it per log request. */
        if (!SetSyslogHost(value))
        {
            Log(LOG_LEVEL_ERR,
                  "Failed to set syslog_host to '%s', too long", (const char *)value);
        }
        else
        {
            Log(LOG_LEVEL_VERBOSE, "Setting syslog_host to '%s'", (const char *)value);
        }
    }

    if ((value = EvalContextVariableControlCommonGet(ctx, COMMON_CONTROL_BWLIMIT)))
    {
        double bval;
        if (DoubleFromString(value, &bval))
        {
            bwlimit_kbytes = (uint32_t) ( bval / 1000.0);
            Log(LOG_LEVEL_VERBOSE, "Setting rate limit to %d kBytes/sec", bwlimit_kbytes);
        }
    }
    Nova_Initialize(ctx);
    Nova_InitializeLeech2();

    // If not have been enabled above then should be disabled.
    // By default it's enabled to catch all set classes on startup stage
    // before this part of the policy is processed.
    if (!config->agent_specific.agent.report_class_log)
    {
        EvalContextAllClassesLoggingEnable(ctx, false);
    }
}

/*********************************************************************/

static void KeepPromiseBundles(EvalContext *ctx, const Policy *policy, GenericAgentConfig *config)
{
    Rlist *bundlesequence = NULL;

    Banner("Begin policy/promise evaluation");

    if (config->bundlesequence != NULL)
    {
        Log(LOG_LEVEL_INFO, "Using command line specified bundlesequence");
        bundlesequence = RlistCopy(config->bundlesequence);
    }
    else
    {
        bundlesequence = RlistCopy((Rlist *) EvalContextVariableControlCommonGet(
                                       ctx, COMMON_CONTROL_BUNDLESEQUENCE));

        if (bundlesequence == NULL)
        {
            RlistAppendScalar(&bundlesequence, "main");
        }
    }

    bool ok = true;
    for (const Rlist *rp = bundlesequence; rp; rp = rp->next)
    {
        const char *name = NULL;

        switch (rp->val.type)
        {
        case RVAL_TYPE_SCALAR:
            name = RlistScalarValue(rp);
            break;
        case RVAL_TYPE_FNCALL:
            name = RlistFnCallValue(rp)->name;
            break;

        default:
            name = NULL;
            {
                Writer *w = StringWriter();
                WriterWrite(w, "Illegal item found in bundlesequence: ");
                RvalWrite(w, rp->val);
                Log(LOG_LEVEL_ERR, "%s", StringWriterData(w));
                WriterClose(w);
            }
            ok = false;
            break;
        }

        if (!config->ignore_missing_bundles)
        {
            const Bundle *bp = EvalContextResolveBundleExpression(ctx, policy, name, "agent");
            if (!bp)
            {
                bp = EvalContextResolveBundleExpression(ctx, policy, name, "common");
            }

            if (!bp)
            {
                Log(LOG_LEVEL_ERR, "Bundle '%s' listed in the bundlesequence was not found", name);
                ok = false;
            }
        }
    }

    if (!ok)
    {
        FatalError(ctx, "Errors in agent bundles");
    }

    Writer *w = StringWriter();
    WriterWrite(w, "Using bundlesequence => ");
    RlistWrite(w, bundlesequence);
    Log(LOG_LEVEL_VERBOSE, "%s", StringWriterData(w));
    WriterClose(w);

/* If all is okay, go ahead and evaluate */

    for (const Rlist *rp = bundlesequence; rp; rp = rp->next)
    {
        const char *name = NULL;
        const Rlist *args = NULL;

        if (rp->val.type == RVAL_TYPE_FNCALL)
        {
            name = RlistFnCallValue(rp)->name;
            args = RlistFnCallValue(rp)->args;
        }
        else
        {
            name = RlistScalarValue(rp);
            args = NULL;
        }

        EvalContextSetBundleArgs(ctx, args);

        const Bundle *bp = EvalContextResolveBundleExpression(ctx, policy, name, "agent");
        if (!bp)
        {
            bp = EvalContextResolveBundleExpression(ctx, policy, name, "common");
        }

        if (bp)
        {
            BundleBanner(bp,args);
            EvalContextStackPushBundleFrame(ctx, bp, args, false, NULL);
            ScheduleAgentOperations(ctx, bp);
            EvalContextStackPopFrame(ctx);
            EndBundleBanner(bp);
            if (EvalAborted(ctx))
            {
                break;
            }
        }
        else
        {
            if (config->ignore_missing_bundles)
            {
                Log(LOG_LEVEL_VERBOSE, "Ignoring missing bundle '%s'", name);
            }
            else
            {
                FatalError(ctx, "Bundlesequence contained unknown bundle reference '%s'", name);
            }
        }
    }

    RlistDestroy(bundlesequence);
}

PromiseResult ScheduleAgentOperations(EvalContext *ctx, const Bundle *bp)
// NB - this function can be called recursively through "methods"
{
    if (EvalContextIsClassicOrder(ctx, bp))
    {
        return ScheduleAgentOperationsNormalOrder(ctx, bp);
    }
    return ScheduleAgentOperationsTopDownOrder(ctx, bp);
}

PromiseResult ScheduleAgentOperationsNormalOrder(EvalContext *ctx, const Bundle *bp)
{
    assert(bp != NULL);

    int save_pr_kept = PR_KEPT;
    int save_pr_repaired = PR_REPAIRED;
    int save_pr_notkept = PR_NOTKEPT;
    struct timespec start = BeginMeasure();

    if (PROCESSREFRESH == NULL || (PROCESSREFRESH && IsRegexItemIn(ctx, PROCESSREFRESH, bp->name)))
    {
        ClearProcessTable();
    }

    PromiseResult result = PROMISE_RESULT_SKIPPED;

    for (int pass = 1; pass < CF_DONEPASSES; pass++)
    {
        // Evaluate built-in (non-custom) promise types, according to type sequence (normal order):
        for (TypeSequence type = 0; AGENT_TYPESEQUENCE[type] != NULL; type++)
        {
            const BundleSection *sp = BundleGetSection((Bundle *)bp, AGENT_TYPESEQUENCE[type]);

            if (!sp || SeqLength(sp->promises) == 0)
            {
                continue;
            }

            NewTypeContext(type);

            SpecialTypeBanner(type, pass);
            EvalContextStackPushBundleSectionFrame(ctx, sp);

            for (size_t ppi = 0; ppi < SeqLength(sp->promises); ppi++)
            {
                Promise *pp = SeqAt(sp->promises, ppi);

                EvalContextSetPass(ctx, pass);

                PromiseResult promise_result = ExpandPromise(ctx, pp, KeepAgentPromise, NULL);
                result = PromiseResultUpdate(result, promise_result);

                if (EvalAborted(ctx) || BundleAbort(ctx))
                {
                    DeleteTypeContext(ctx, type);
                    EvalContextStackPopFrame(ctx);
                    NoteBundleCompliance(bp, save_pr_kept, save_pr_repaired, save_pr_notkept, start);
                    return result;
                }
            }

            DeleteTypeContext(ctx, type);
            EvalContextStackPopFrame(ctx);

            if (type == TYPE_SEQUENCE_CONTEXTS)
            {
                BundleResolve(ctx, bp);
                BundleResolvePromiseType(ctx, bp, "defaults", DefaultVarPromiseWrapper);
            }
        }

        // Custom promises are evaluated at the end of an evaluation pass:
        const size_t sections = SeqLength(bp->custom_sections);
        for (size_t i = 0; i < sections; ++i)
        {
            BundleSection *section = SeqAt(bp->custom_sections, i);

            EvalContextStackPushBundleSectionFrame(ctx, section);

            const size_t promises = SeqLength(section->promises);
            for (size_t ppi = 0; ppi < promises; ppi++)
            {
                Promise *pp = SeqAt(section->promises, ppi);

                EvalContextSetPass(ctx, pass);

                PromiseResult promise_result = ExpandPromise(ctx, pp, KeepAgentPromise, NULL);
                result = PromiseResultUpdate(result, promise_result);

                if (EvalAborted(ctx) || BundleAbort(ctx))
                {
                    EvalContextStackPopFrame(ctx);
                    NoteBundleCompliance(bp, save_pr_kept, save_pr_repaired, save_pr_notkept, start);
                    return result;
                }
            }
            EvalContextStackPopFrame(ctx);
        }
    }

    NoteBundleCompliance(bp, save_pr_kept, save_pr_repaired, save_pr_notkept, start);
    return result;
}

PromiseResult ScheduleAgentOperationsTopDownOrder(EvalContext *ctx, const Bundle *bp)
{
    assert(bp != NULL);

    int save_pr_kept = PR_KEPT;
    int save_pr_repaired = PR_REPAIRED;
    int save_pr_notkept = PR_NOTKEPT;
    struct timespec start = BeginMeasure();

    if (PROCESSREFRESH == NULL || (PROCESSREFRESH && IsRegexItemIn(ctx, PROCESSREFRESH, bp->name)))
    {
        ClearProcessTable();
    }

    PromiseResult result = PROMISE_RESULT_SKIPPED;
    for (int pass = 1; pass < CF_DONEPASSES; pass++)
    {
        const char *last_promise_type = "";
        for (size_t ppi = 0; ppi < SeqLength(bp->all_promises); ppi++)
        {
            EvalContextSetPass(ctx, pass);
            Promise *pp = SeqAt(bp->all_promises, ppi);
            BundleSection *parent_section = pp->parent_section;

            if (!StringEqual(last_promise_type, parent_section->promise_type))
            {
                SpecialTypeBannerFromString(parent_section->promise_type, pass);
            }
            last_promise_type = parent_section->promise_type;

            EvalContextStackPushBundleSectionFrame(ctx, parent_section);

            PromiseResult promise_result = ExpandPromise(ctx, pp, KeepAgentPromise, NULL);
            result = PromiseResultUpdate(result, promise_result);
            if (EvalAborted(ctx) || BundleAbort(ctx))
            {
                EvalContextStackPopFrame(ctx);
                NoteBundleCompliance(bp, save_pr_kept, save_pr_repaired, save_pr_notkept, start);
                return result;
            }
            EvalContextStackPopFrame(ctx);
        }
    }

    NoteBundleCompliance(bp, save_pr_kept, save_pr_repaired, save_pr_notkept, start);
    return result;
}

/*********************************************************************/

#ifdef __MINGW32__

static void CheckAgentAccess(const Rlist *list, const Policy *policy)
{
}

#else

static void CheckAgentAccess(const Rlist *list, const Policy *policy)
{
    uid_t uid = getuid();

    for (const Rlist *rp = list; rp != NULL; rp = rp->next)
    {
        if (Str2Uid(RlistScalarValue(rp), NULL, 0, NULL) == uid)
        {
            return;
        }
    }

    {
        StringSet *input_files = PolicySourceFiles(policy);
        StringSetIterator iter = StringSetIteratorInit(input_files);
        const char *input_file = NULL;
        while ((input_file = StringSetIteratorNext(&iter)))
        {
            struct stat sb;
            stat(input_file, &sb);

            if (ACCESSLIST)
            {
                bool access = false;
                for (const Rlist *rp2 = ACCESSLIST; rp2 != NULL; rp2 = rp2->next)
                {
                    if (Str2Uid(RlistScalarValue(rp2), NULL, 0, NULL) == sb.st_uid)
                    {
                        access = true;
                        break;
                    }
                }

                if (!access)
                {
                    Log(LOG_LEVEL_ERR, "File '%s' is not owned by an authorized user (security exception)", input_file);
                    DoCleanupAndExit(EXIT_FAILURE);
                }
            }
            else if (CFPARANOID && IsPrivileged())
            {
                if (sb.st_uid != getuid())
                {
                    Log(LOG_LEVEL_ERR, "File '%s' is not owned by uid %ju (security exception)", input_file,
                          (uintmax_t)getuid());
                    DoCleanupAndExit(EXIT_FAILURE);
                }
            }
        }

        StringSetDestroy(input_files);
    }

    Log(LOG_LEVEL_ERR, "You are denied access to run this policy");
    DoCleanupAndExit(EXIT_FAILURE);
}
#endif /* !__MINGW32__ */

/*********************************************************************/

static PromiseResult DefaultVarPromise(EvalContext *ctx, const Promise *pp)
{
    char *regex = PromiseGetConstraintAsRval(pp, "if_match_regex", RVAL_TYPE_SCALAR);
    bool okay = true;


    DataType value_type = CF_DATA_TYPE_NONE;
    const void *value = NULL;
    {
        VarRef *ref = VarRefParseFromScope(pp->promiser, "this");
        value = EvalContextVariableGetPlaintext(ctx, ref, &value_type);
        VarRefDestroy(ref);
    }

    switch (value_type)
    {
    case CF_DATA_TYPE_STRING:
    case CF_DATA_TYPE_INT:
    case CF_DATA_TYPE_REAL:
        if (regex && !FullTextMatch(ctx, regex, value))
        {
            return PROMISE_RESULT_NOOP;
        }

        if (regex == NULL)
        {
            return PROMISE_RESULT_NOOP;
        }
        break;

    case CF_DATA_TYPE_STRING_LIST:
    case CF_DATA_TYPE_INT_LIST:
    case CF_DATA_TYPE_REAL_LIST:
        if (regex)
        {
            for (const Rlist *rp = value; rp != NULL; rp = rp->next)
            {
                if (FullTextMatch(ctx, regex, RlistScalarValue(rp)))
                {
                    okay = false;
                    break;
                }
            }

            if (okay)
            {
                return PROMISE_RESULT_NOOP;
            }
        }
        break;

    default:
        break;
    }

    {
        VarRef *ref = VarRefParseFromBundle(pp->promiser, PromiseGetBundle(pp));
        EvalContextVariableRemove(ctx, ref);
        VarRefDestroy(ref);
    }

    return VerifyVarPromise(ctx, pp, NULL);
}

static void LogVariableValue(const EvalContext *ctx, const Promise *pp)
{
    VarRef *ref = VarRefParseFromBundle(pp->promiser, PromiseGetBundle(pp));
    char *out = NULL;

    DataType type;
    const void *var = EvalContextVariableGetPlaintext(ctx, ref, &type);
    switch (type)
    {
        case CF_DATA_TYPE_INT:
        case CF_DATA_TYPE_REAL:
        case CF_DATA_TYPE_STRING:
            out = xstrdup((char *) var);
            break;
        case CF_DATA_TYPE_INT_LIST:
        case CF_DATA_TYPE_REAL_LIST:
        case CF_DATA_TYPE_STRING_LIST:
        {
            size_t siz = CF_BUFSIZE;
            size_t len = 0;
            out = xcalloc(1, CF_BUFSIZE);

            for (Rlist *rp = (Rlist *) var; rp != NULL; rp = rp->next)
            {
                const char *s = (char *) rp->val.item;

                if (strlen(s) + len + 3  >= siz)                // ", " + NULL
                {
                    out = xrealloc(out, siz + CF_BUFSIZE);
                    siz += CF_BUFSIZE;
                }

                if (len > 0)
                {
                    len += strlcat(out, ", ", siz);
                }

                len += strlcat(out, s, siz);
            }
            break;
        }
        case CF_DATA_TYPE_CONTAINER:
        {
            Writer *w = StringWriter();
            JsonWriteCompact(w, (JsonElement *) var);
            out = StringWriterClose(w);
            break;
        }
        default:
            /* TODO is CF_DATA_TYPE_NONE acceptable? Today all meta variables
             * are of this type. */
            /* UnexpectedError("Variable '%s' is of unknown type %d", */
            /*                 pp->promiser, type); */
            out = xstrdup("NONE");
            break;
    }

    Log(LOG_LEVEL_DEBUG, "V: '%s' => '%s'", pp->promiser, out);
    free(out);
    VarRefDestroy(ref);
}

static PromiseResult KeepAgentPromise(EvalContext *ctx, const Promise *pp, ARG_UNUSED void *param)
{
    assert(param == NULL);
    assert(pp != NULL);

    BannerStatusBegin(PromiseGetPromiseType(pp), pp->promiser);
    struct timespec start = BeginMeasure();
    PromiseResult result = PROMISE_RESULT_NOOP;

    if (strcmp("meta", PromiseGetPromiseType(pp)) == 0 ||
        strcmp("vars", PromiseGetPromiseType(pp)) == 0)
    {
        Log(LOG_LEVEL_VERBOSE, "V:     Computing value of '%s'", pp->promiser);

        result = VerifyVarPromise(ctx, pp, NULL);
        if (result != PROMISE_RESULT_FAIL)
        {
            if (LogGetGlobalLevel() >= LOG_LEVEL_DEBUG)
            {
                LogVariableValue(ctx, pp);
            }
        }
    }
    else if (strcmp("defaults", PromiseGetPromiseType(pp)) == 0)
    {
        result = DefaultVarPromise(ctx, pp);
    }
    else if (strcmp("classes", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyClassPromise(ctx, pp, NULL);
    }
    else if (strcmp("processes", PromiseGetPromiseType(pp)) == 0)
    {
        if (!LoadProcessTable())
        {
            Log(LOG_LEVEL_ERR, "Unable to read the process table - cannot keep processes: type promises");
            return PROMISE_RESULT_FAIL;
        }
        result = VerifyProcessesPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("storage", PromiseGetPromiseType(pp)) == 0)
    {
        result = FindAndVerifyStoragePromises(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("packages", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyPackagesPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("users", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyUsersPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }

    else if (strcmp("files", PromiseGetPromiseType(pp)) == 0)
    {
        result = ParallelFindAndVerifyFilesPromises(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("commands", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyExecPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("databases", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyDatabasePromises(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("methods", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyMethodsPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("services", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyServicesPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("guest_environments", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyEnvironmentsPromise(ctx, pp);
        if (result != PROMISE_RESULT_SKIPPED)
        {
            EndMeasurePromise(start, pp);
        }
    }
    else if (strcmp("reports", PromiseGetPromiseType(pp)) == 0)
    {
        result = VerifyReportPromise(ctx, pp);
    }
    else if (!IsBuiltInPromiseType(PromiseGetPromiseType(pp)))
    {
        result = EvaluateCustomPromise(ctx, pp);
    }
    else
    {
        result = PROMISE_RESULT_NOOP;
    }

    BannerStatusEnd(result, PromiseGetPromiseType(pp), pp->promiser);
    EvalContextLogPromiseIterationOutcome(ctx, pp, result);
    return result;
}

static void BannerStatusBegin(const char *type, char *name)
{
    if (StringEqual(type, "vars") || StringEqual(type, "classes"))
    {
        return;
    }
    Log(LOG_LEVEL_VERBOSE, "P: BEGIN %s promise (%.30s%s)",
        type, name,
        (strlen(name) > 30) ? "..." : "");
}

static void BannerStatusEnd(PromiseResult status, const char *type, char *name)
{
    if ((strcmp(type, "vars") == 0) || (strcmp(type, "classes") == 0))
    {
        return;
    }

    switch (status)
    {
    case PROMISE_RESULT_CHANGE:
        Log(LOG_LEVEL_VERBOSE, "A: Promise REPAIRED");
        break;

    case PROMISE_RESULT_TIMEOUT:
        Log(LOG_LEVEL_VERBOSE, "A: Promise TIMED-OUT");
        break;

    case PROMISE_RESULT_WARN:
    case PROMISE_RESULT_FAIL:
    case PROMISE_RESULT_INTERRUPTED:
        Log(LOG_LEVEL_VERBOSE, "A: Promise NOT KEPT!");
        break;

    case PROMISE_RESULT_DENIED:
        Log(LOG_LEVEL_VERBOSE, "A: Promise NOT KEPT - denied");
        break;

    case PROMISE_RESULT_NOOP:
        Log(LOG_LEVEL_VERBOSE, "A: Promise was KEPT");
        break;
    default:
        return;
        break;
    }

    Log(LOG_LEVEL_VERBOSE, "P: END %s promise (%.30s%s)",
        type, name,
        (strlen(name) > 30) ? "..." : "");
}

/*********************************************************************/
/* Type context                                                      */
/*********************************************************************/

static void NewTypeContext(TypeSequence type)
{
// get maxconnections

    switch (type)
    {
    case TYPE_SEQUENCE_ENVIRONMENTS:
        NewEnvironmentsContext();
        break;

    case TYPE_SEQUENCE_FILES:
        break;

    case TYPE_SEQUENCE_PROCESSES:
        break;

    case TYPE_SEQUENCE_STORAGE:
#ifndef __MINGW32__                   // TODO: Run if implemented on Windows
        if (SeqLength(GetGlobalMountedFSList()))
        {
            DeleteMountInfo(GetGlobalMountedFSList());
            SeqClear(GetGlobalMountedFSList());
        }
#endif /* !__MINGW32__ */
        break;

    default:
        break;
    }

    return;
}

/*********************************************************************/

static void DeleteTypeContext(EvalContext *ctx, TypeSequence type)
{
    switch (type)
    {
    case TYPE_SEQUENCE_ENVIRONMENTS:
        DeleteEnvironmentsContext();
        break;

    case TYPE_SEQUENCE_FILES:
        break;

    case TYPE_SEQUENCE_PROCESSES:
        break;

    case TYPE_SEQUENCE_STORAGE:
        DeleteStorageContext();
        break;

    case TYPE_SEQUENCE_PACKAGES:
        ExecuteScheduledPackages(ctx);
        CleanScheduledPackages();
        break;

    default:
        break;
    }
}

/**************************************************************/
/* Thread context                                             */
/**************************************************************/

#ifdef __MINGW32__

static PromiseResult ParallelFindAndVerifyFilesPromises(EvalContext *ctx, const Promise *pp)
{
    int background = PromiseGetConstraintAsBoolean(ctx, "background", pp);

    if (background)
    {
        Log(LOG_LEVEL_VERBOSE, "Background processing of files promises is not supported on Windows");
    }

    return FindAndVerifyFilesPromises(ctx, pp);
}

#else /* !__MINGW32__ */

static PromiseResult ParallelFindAndVerifyFilesPromises(EvalContext *ctx, const Promise *pp)
{
    int background = PromiseGetConstraintAsBoolean(ctx, "background", pp);
    pid_t child = 1;
    PromiseResult result = PROMISE_RESULT_SKIPPED;

    if (background)
    {
        if (CFA_BACKGROUND < CFA_BACKGROUND_LIMIT)
        {
            CFA_BACKGROUND++;
            Log(LOG_LEVEL_VERBOSE, "Spawning new process...");
            child = fork();

            if (child == 0)
            {
                ALARM_PID = -1;

                result = PromiseResultUpdate(result, FindAndVerifyFilesPromises(ctx, pp));

                Log(LOG_LEVEL_VERBOSE, "Exiting backgrounded promise");
                PromiseRef(LOG_LEVEL_VERBOSE, pp);
                _exit(EXIT_SUCCESS);
                // TODO: need to solve this
            }
        }
        else
        {
            Log(LOG_LEVEL_VERBOSE, "Promised parallel execution promised but exceeded the max number of promised background tasks, so serializing");
            background = 0;
        }
    }
    else
    {
        result = PromiseResultUpdate(result, FindAndVerifyFilesPromises(ctx, pp));
    }

    return result;
}

#endif /* !__MINGW32__ */

/**************************************************************/
/* Compliance comp                                            */
/**************************************************************/

static int NoteBundleCompliance(const Bundle *bundle, int save_pr_kept, int save_pr_repaired, int save_pr_notkept, struct timespec start)
{
    double delta_pr_kept, delta_pr_repaired, delta_pr_notkept;
    double bundle_compliance = 0.0;

    delta_pr_kept = (double) (PR_KEPT - save_pr_kept);
    delta_pr_notkept = (double) (PR_NOTKEPT - save_pr_notkept);
    delta_pr_repaired = (double) (PR_REPAIRED - save_pr_repaired);

    Log(LOG_LEVEL_VERBOSE, "A: ...................................................");
    Log(LOG_LEVEL_VERBOSE, "A: Bundle Accounting Summary for '%s' in namespace %s", bundle->name, bundle->ns);

    if (delta_pr_kept + delta_pr_notkept + delta_pr_repaired <= 0)
    {
        Log(LOG_LEVEL_VERBOSE, "A: Zero promises executed for bundle '%s'", bundle->name);
        Log(LOG_LEVEL_VERBOSE, "A: ...................................................");
        return PROMISE_RESULT_NOOP;
    }
    else
    {
        Log(LOG_LEVEL_VERBOSE, "A: Promises kept in '%s' = %.0lf", bundle->name, delta_pr_kept);
        Log(LOG_LEVEL_VERBOSE, "A: Promises not kept in '%s' = %.0lf", bundle->name, delta_pr_notkept);
        Log(LOG_LEVEL_VERBOSE, "A: Promises repaired in '%s' = %.0lf", bundle->name, delta_pr_repaired);

        bundle_compliance = (delta_pr_kept + delta_pr_repaired) / (delta_pr_kept + delta_pr_notkept + delta_pr_repaired);

        Log(LOG_LEVEL_VERBOSE, "A: Aggregate compliance (promises kept/repaired) for bundle '%s' = %.1lf%%",
          bundle->name, bundle_compliance * 100.0);

        if (LogGetGlobalLevel() >= LOG_LEVEL_INFO)
        {
            char name[CF_MAXVARSIZE];
            snprintf(name, CF_MAXVARSIZE, "%s:%s", bundle->ns, bundle->name);
            EndMeasure(name, start);
        }
        else
        {
            EndMeasure(NULL, start);
        }
        Log(LOG_LEVEL_VERBOSE, "A: ...................................................");
    }

    // return the worst case for the bundle status

    if (delta_pr_notkept > 0)
    {
        return PROMISE_RESULT_FAIL;
    }

    if (delta_pr_repaired > 0)
    {
        return PROMISE_RESULT_CHANGE;
    }

    return PROMISE_RESULT_NOOP;
}
