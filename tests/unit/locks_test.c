#include <test.h>

#include <locks.h>
#include <eval_context.h> // EvalContext
#include <policy.h> // Promise_
#include <string_lib.h> // StringBytesToHex
#include <rlist.h> // RlistParseString

static void test_PromiseRuntimeHash()
{
    // create Promise like iteration_test.c
    /* INIT EvalContext and Promise. */
    const char *promiser = "test-promiser";

    EvalContext *evalctx = EvalContextNew();
    Policy *policy = PolicyNew();
    Bundle *bundle = PolicyAppendBundle(policy, "ns1", "bundle1", "agent",
                                        NULL, NULL);
    PromiseType *promise_type = BundleAppendPromiseType(bundle, "processes");
    Promise *promise = PromiseTypeAppendPromise(promise_type, promiser,
                                                (Rval) { NULL, RVAL_TYPE_NOPROMISEE },
                                                "any", NULL);

      // add (d) userargs to the body
    Rlist *userargs = NULL;
    //RlistPrepend(&userargs, "d", RVAL_TYPE_SCALAR);

      // add process_select body with stime_range and process_result constraints
      Body *process_select_body = PolicyAppendBody(policy, "", "days_older_than", "process_select", userargs, "fake-source-path");

      Rlist *irange_args = NULL;
      // but I want not a scalar "now" but a variable reference or something
      RlistAppendRval(&irange_args, RvalNew("10", RVAL_TYPE_SCALAR));
      RlistAppendRval(&irange_args, RvalNew("20", RVAL_TYPE_SCALAR));
      FnCall *irange = FnCallNew("irange", irange_args);
      BodyAppendConstraint(process_select_body, "stime_range", (Rval) { irange, RVAL_TYPE_FNCALL }, "any", false);
      BodyAppendConstraint(process_select_body, "process_result", RvalNew("!stime", RVAL_TYPE_SCALAR), "any", false);

      FnCall *days_older_than_call = FnCallNew("days_older_than", NULL);
      PromiseAppendConstraint(promise, "process_select", (Rval) { days_older_than_call, RVAL_TYPE_FNCALL }, true);
      // add process_select body to promise as a constraint
      //      FnCall *days_older_than = FnCallNew("days_older_than", NULL);
      //      PromiseAppendConstraint(promise, "ifvarclass", RvalNew(varclasses, RVAL_TYPE_SCALAR), true);

      
    
    EvalContextStackPushBundleFrame(evalctx, bundle, NULL, false);
    EvalContextStackPushPromiseTypeFrame(evalctx, promise_type);

    // print out the policy to see what it is
    Writer *writer = FileWriter(stdout);
    PolicyToString(policy, writer);
    
    // generate promise hash
    unsigned char digest[EVP_MAX_MD_SIZE + 1];
    PromiseRuntimeHash(promise, "", digest, HASH_METHOD_SHA256);

    char promise_hash[(2 * CF_SHA256_LEN) + 1];
    StringBytesToHex(promise_hash, sizeof(promise_hash),
                         digest, CF_SHA256_LEN);

    printf("promise_hash: %s\n", promise_hash);
    assert_string_equal(promise_hash, "12f3badf92f46ea72add0803c89481cf7a5da76aac826df44bbf59487ceab54a");

    WriterClose(writer);
}

int main()
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] =
    {
         unit_test(test_PromiseRuntimeHash)
    };

    return run_tests(tests);
}
