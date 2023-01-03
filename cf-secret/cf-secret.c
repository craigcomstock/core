/*
   Copyright 2023 Northern.tech AS

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

/*
   cf-secret.c

   Copyright (C) 2017 cfengineers.net

   Written and maintained by Jon Henrik Bjornstad <jonhenrik@cfengineers.net>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include <platform.h>
#include <openssl/err.h>

#include <lastseen.h>
#include <crypto.h>
#include <generic_agent.h> /* GenericAgentSetDefaultDigest */
#include <writer.h>
#include <man.h>
#include <conversion.h>
#include <hash.h>
#include <known_dirs.h>
#include <string_lib.h>
#include <file_lib.h>
#include <sequence.h>
#include <string_sequence.h>
#include <unistd.h>
#include <cleanup.h>            /* DoCleanupAndExit(), CallCleanupFunctions() */
#include <ip_address.h>

#define BUFSIZE 1024

#define MAX_HEADER_LEN 256      /* "Key[126]: Value[128]" */
#define MAX_HEADER_KEY_LEN 126
#define MAX_HEADER_VAL_LEN 128

typedef enum {
    HOST_RSA_KEY_PRIVATE,
    HOST_RSA_KEY_PUBLIC,
} HostRSAKeyType;

/* see README.md for details about the format */
typedef enum {
    CF_SECRET_FORMAT_V_1_0,
} CFKeyCryptFormatVersion;

static const char passphrase[] = "Cfengine passphrase";

//*******************************************************************
// DOCUMENTATION / GETOPT CONSTS:
//*******************************************************************

static const char *const CF_SECRET_SHORT_DESCRIPTION =
    "cf-secret: Use CFEngine cryptographic keys to encrypt and decrypt files";

static const char *const CF_SECRET_MANPAGE_LONG_DESCRIPTION =
    "cf-secret offers a simple way to encrypt or decrypt files using keys "
    "generated by cf-key. CFEngine uses asymmetric cryptography, and "
    "cf-secret allows you to encrypt a file using a public key file. "
    "The encrypted file can only be decrypted on the host with the "
    "corresponding private key. Original author: Jon Henrik Bjornstad "
    "<jonhenrik@cfengineers.net>";

static const struct option OPTIONS[] =
{
    {"help",        no_argument,        0, 'h'},
    {"manpage",     no_argument,        0, 'M'},
    {"debug",       no_argument,        0, 'd'},
    {"verbose",     no_argument,        0, 'v'},
    {"log-level",   required_argument,  0, 'g'},
    {"inform",      no_argument,        0, 'I'},
    {"key",         required_argument,  0, 'k'},
    {"host",        required_argument,  0, 'H'},
    {"output",      required_argument,  0, 'o'},
    {NULL,          0,                  0, '\0'}
};

static const char *const HINTS[] =
{
    "Print the help message",
    "Print the man page",
    "Enable debugging output",
    "Enable verbose output",
    "Specify how detailed logs should be. Possible values: 'error', 'warning', 'notice', 'info', 'verbose', 'debug'",
    "Enable basic information output",
    "Comma-separated list of key files to use (one of -k/-H options is required for encryption)",
    "Comma-separated list of hosts to encrypt/decrypt for (defaults to 'localhost' for decryption)",
    "Output file (required)",
    NULL
};

static const Description COMMANDS[] =
{
    {"encrypt", "Encrypt data for one or more hosts/keys", "cf-secret encrypt -k/-H KEY/HOST -o OUTPUT INPUT"},
    {"decrypt", "Decrypt data", "cf-secret decrypt [-k/-H KEY/HOST] -o OUTPUT INPUT"},
    {"print-headers", "Print headers from an encrypted file", "cf-secret print-headers ENCRYPTED_FILE"},
    {NULL, NULL, NULL}
};

static inline void *GetIPAddress(struct sockaddr *sa)
{
    assert(sa != NULL);
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * Get path of the RSA key for the given host.
 */
static char *GetHostRSAKey(const char *host, HostRSAKeyType type)
{
    const char *key_ext = NULL;
    if (type == HOST_RSA_KEY_PRIVATE)
    {
        key_ext = ".priv";
    }
    else
    {
        key_ext = ".pub";
    }

    struct addrinfo *result;
    int error = getaddrinfo(host, NULL, NULL, &result);
    if (error != 0)
    {
        Log(LOG_LEVEL_ERR, "Failed to get IP from host (getaddrinfo: %s)",
            gai_strerror(error));
        return NULL;
    }

    char *buffer = malloc(PATH_MAX);
    char hash[CF_HOSTKEY_STRING_SIZE];
    char ipaddress[64];
    bool found = false;
    for (struct addrinfo *res = result; !found && (res != NULL); res = res->ai_next)
    {
        inet_ntop(res->ai_family,
                  GetIPAddress((struct sockaddr *) res->ai_addr),
                  ipaddress, sizeof(ipaddress));
        if (StringIsLocalHostIP(ipaddress))
        {
            Log(LOG_LEVEL_VERBOSE, "Using localhost%s key", key_ext);
            found = true;
            int ret = snprintf(buffer, PATH_MAX, "%s/ppkeys/localhost%s",
                      GetWorkDir(), key_ext);
            if (ret < 0 || ret >= PATH_MAX)
            {
                Log(LOG_LEVEL_ERR, "Path to RSA key is too long (%d > %d)",
                    ret, PATH_MAX - 1);
                freeaddrinfo(result);
                return NULL;
            }
            freeaddrinfo(result);
            return buffer;
        }
        found = Address2Hostkey(hash, sizeof(hash), ipaddress);
    }
    if (found)
    {
        Log(LOG_LEVEL_DEBUG, "Found host '%s' for address '%s'", hash, ipaddress);
        int ret = snprintf(buffer, PATH_MAX, "%s/ppkeys/root-%s%s",
                           GetWorkDir(), hash, key_ext);
        if (ret < 0 || ret >= PATH_MAX)
        {
            Log(LOG_LEVEL_ERR, "Path to RSA key is too long (%d > %d)", ret,
                PATH_MAX - 1);
            freeaddrinfo(result);
            return NULL;
        }
        freeaddrinfo(result);
        return buffer;
    }
    else
    {
        Log(LOG_LEVEL_DEBUG, "Searching key by IP");
        for (struct addrinfo *res = result; res != NULL; res = res->ai_next)
        {
            inet_ntop(res->ai_family,
                      GetIPAddress((struct sockaddr *) res->ai_addr),
                      ipaddress, sizeof(ipaddress));
            int ret = snprintf(buffer, BUFSIZE, "%s/ppkeys/root-%s%s",
                               GetWorkDir(), ipaddress, key_ext);
            if (ret < 0 || ret >= PATH_MAX)
            {
                Log(LOG_LEVEL_ERR, "Path to RSA key is too long (%d > %d)",
                    ret, PATH_MAX - 1);
                freeaddrinfo(result);
                return NULL;
            }
            if (access(buffer, F_OK) == 0)
            {
                Log(LOG_LEVEL_DEBUG, "Found matching key: '%s'", buffer);
                freeaddrinfo(result);
                return buffer;
            }
        }
    }
    freeaddrinfo(result);
    return NULL;
}

static RSA *ReadPrivateKey(const char *privkey_path)
{
    FILE *fp = safe_fopen(privkey_path,"r");

    if (fp == NULL)
    {
        Log(LOG_LEVEL_ERR, "Could not open private key '%s'", privkey_path);
        return NULL;
    }
    RSA *privkey = PEM_read_RSAPrivateKey(fp, (RSA **) NULL, NULL, (void *) passphrase);
    if (privkey == NULL)
    {
        unsigned long err = ERR_get_error();
        Log(LOG_LEVEL_ERR, "Could not read private key '%s': %s",
            privkey_path, ERR_reason_error_string(err));
    }
    fclose(fp);
    return privkey;
}

static RSA *ReadPublicKey(const char *pubkey_path)
{
    FILE *fp = safe_fopen(pubkey_path, "r");

    if (fp == NULL)
    {
        Log(LOG_LEVEL_ERR, "Could not open public key '%s'", pubkey_path);
        return NULL;
    }

    RSA *pubkey = PEM_read_RSAPublicKey(fp, NULL, NULL, (void *) passphrase);
    if (pubkey == NULL)
    {
        unsigned long err = ERR_get_error();
        Log(LOG_LEVEL_ERR, "Could not read public key '%s': %s",
            pubkey_path, ERR_reason_error_string(err));
    }
    fclose(fp);
    return pubkey;
}

static FILE *OpenInputOutput(const char *path, const char *mode)
{
    assert(path != NULL);
    assert(mode != NULL);
    if (StringEqual(path, "-"))
    {
        if (*mode == 'r')
        {
            return stdin;
        }
        else
        {
            return stdout;
        }
    }
    return safe_fopen(path, mode);
}

static bool RSAEncrypt(Seq *rsa_keys, const char *input_path, const char *output_path)
{
    assert((rsa_keys != NULL) && (SeqLength(rsa_keys) > 0));

    FILE *input_file = OpenInputOutput(input_path, "r");
    if (input_file == NULL)
    {
        Log(LOG_LEVEL_ERR, "Could not open input file '%s'", input_path);
        return false;
    }

    FILE *output_file = OpenInputOutput(output_path, "w");
    if (output_file == NULL)
    {
        Log(LOG_LEVEL_ERR, "Could not create or open output file '%s'", output_path);
        fclose(input_file);
        return false;
    }

    const size_t n_keys = SeqLength(rsa_keys);
    Seq *evp_keys = SeqNew(n_keys, EVP_PKEY_free);
    for (size_t i = 0; i < n_keys; i++)
    {
        RSA *rsa_key = SeqAt(rsa_keys, i);
        EVP_PKEY *evp_key = EVP_PKEY_new();
        if (EVP_PKEY_set1_RSA(evp_key, rsa_key) == 0)
        {
            Log(LOG_LEVEL_ERR, "Failed to initialize encryption context");
            SeqDestroy(evp_keys);
            fclose(input_file);
            fclose(output_file);
            return false;
        }
        SeqAppend(evp_keys, evp_key);
    }

    bool success = true;

    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    const int key_size = EVP_PKEY_size((EVP_PKEY*) SeqAt(evp_keys, 0));

    /* This sequence and the 'enc_key_sizes' array are both populated by the
     * EVP_SealInit() call below. */
    Seq *enc_keys = SeqNew(n_keys, free);
    for (size_t i = 0; i < n_keys; i++)
    {
        SeqAppend(enc_keys, xmalloc(key_size));
    }
    int enc_key_sizes[n_keys];

    const int iv_size = EVP_CIPHER_iv_length(cipher);
    unsigned char iv[iv_size];

    const int block_size = EVP_CIPHER_block_size(cipher);
    char plaintext[block_size], ciphertext[2 * block_size];
    int ct_len;

    int ret = EVP_SealInit(ctx, cipher,
                           (unsigned char**) SeqGetData(enc_keys), enc_key_sizes, iv,
                           (EVP_PKEY**) SeqGetData(evp_keys), n_keys);
    if (ret == 0)
    {
        Log(LOG_LEVEL_ERR, "Failed to initialize encryption context");
        success = false;
        goto cleanup;
    }

    /* newline and NUL-byte => +2 */
    Log(LOG_LEVEL_VERBOSE, "Writing headers");
    char header[MAX_HEADER_LEN + 2] = "Version: 1.0\n";
    size_t header_len = strlen(header);
    ssize_t n_written = FullWrite(fileno(output_file), header, header_len);
    if (n_written < 0 || (size_t) n_written != header_len)
    {
        Log(LOG_LEVEL_ERR, "Failed to write header to the output file '%s'", output_path);
        success = false;
        goto cleanup;
    }
    Log(LOG_LEVEL_VERBOSE, "Writing Encrypted-for headers");
    for (size_t i = 0; i < n_keys; i++)
    {
        char *key_digest = GetPubkeyDigest(SeqAt(rsa_keys, i));
        header_len = snprintf(header, MAX_HEADER_LEN + 2, "Encrypted-for: %s\n", key_digest);
        free(key_digest);
        assert(header_len <= (MAX_HEADER_LEN + 2));
        n_written = FullWrite(fileno(output_file), header, header_len);
        if ((size_t) n_written != header_len)
        {
            Log(LOG_LEVEL_ERR, "Failed to write header to the output file '%s'", output_path);
            success = false;
            goto cleanup;
        }
    }
    n_written = FullWrite(fileno(output_file), "\n", 1);
    if (n_written != 1)
    {
        Log(LOG_LEVEL_ERR, "Failed to write header to the output file '%s'", output_path);
        success = false;
        goto cleanup;
    }
    Log(LOG_LEVEL_VERBOSE, "Writing IV");
    n_written = FullWrite(fileno(output_file), iv, iv_size);
    if (n_written != iv_size)
    {
        Log(LOG_LEVEL_ERR, "Failed to write IV to the output file '%s'", output_path);
        success = false;
        goto cleanup;
    }

    Log(LOG_LEVEL_VERBOSE, "Writing keys");
    for (size_t i = 0; i < n_keys; i++)
    {
        const char *enc_key = SeqAt(enc_keys, i);
        n_written = FullWrite(fileno(output_file), enc_key, enc_key_sizes[i]);
        if (n_written != enc_key_sizes[i])
        {
            Log(LOG_LEVEL_ERR, "Failed to write key to the output file '%s'", output_path);
            success = false;
            goto cleanup;
        }
    }

    size_t processed = 0;
    while (success && !feof(input_file))
    {
        ssize_t n_read = ReadFileStreamToBuffer(input_file, block_size, plaintext);
        if (n_read == FILE_ERROR_READ)
        {
            Log(LOG_LEVEL_ERR, "Could not read file '%s'", input_path);
            success = false;
            break;
        }
        ret = EVP_SealUpdate(ctx, ciphertext, &ct_len, plaintext, n_read);
        if (ret == 0)
        {
            Log(LOG_LEVEL_ERR, "Failed to encrypt data: %s",
                ERR_error_string(ERR_get_error(), NULL));
            success = false;
            break;
        }
        n_written = FullWrite(fileno(output_file), ciphertext, ct_len);
        if (n_written < 0)
        {
            Log(LOG_LEVEL_ERR, "Could not write file '%s'", output_path);
            success = false;
            break;
        }
        processed += n_read;
        Log(LOG_LEVEL_VERBOSE, "%zu bytes processed", processed);
    }
    Log(LOG_LEVEL_VERBOSE, "Finalizing");
    ret = EVP_SealFinal(ctx, ciphertext, &ct_len);
    if (ret == 0)
    {
        Log(LOG_LEVEL_ERR, "Failed to encrypt data: %s",
            ERR_error_string(ERR_get_error(), NULL));
        success = false;
    }
    if (ct_len > 0)
    {
        n_written = FullWrite(fileno(output_file), ciphertext, ct_len);
        if (n_written < 0)
        {
            Log(LOG_LEVEL_ERR, "Could not write file '%s'", output_path);
            success = false;
        }
    }
    OPENSSL_cleanse(plaintext, block_size);

  cleanup:
    fclose(input_file);
    fclose(output_file);
    SeqDestroy(evp_keys);
    SeqDestroy(enc_keys);
    EVP_CIPHER_CTX_free(ctx);
    return success;
}

static inline bool CheckHeader(const char *key, const char *value)
{
    if (StringEqual(key, "Version"))
    {
        if (!StringEqual(value, "1.0"))
        {
            Log(LOG_LEVEL_ERR, "Unsupported file format version: '%s'", value);
            return false;
        }
        else
        {
            return true;
        }
    }
    else if (StringEqual(key, "Encrypted-for"))
    {
        /* TODO: do some verification that 'value' is valid hash digest? */
        return true;
    }
    else
    {
        Log(LOG_LEVEL_ERR, "Unsupported header: '%s'", key);
        return false;
    }
}

/**
 * Read from #input_file into #buffer at most #max_chars or until #delimiter is
 * encountered. If #stop_on_space, also stop when ' ' is read. #buffer is
 * NUL-terminated when this function returns.
 *
 * @warning #buffer has to be at least #max_chars+1 big to accommodate for the
 *          terminating NUL byte.
 */
static inline bool ReadUntilDelim(FILE *input_file, char *buffer, size_t max_chars, char delimiter, bool stop_on_space)
{
    bool done = false;
    size_t i = 0;
    int c = fgetc(input_file);
    while (!done && (i < max_chars))
    {
        if (c == EOF)
        {
            done = true;
        }
        else if (c == delimiter)
        {
            done = true;
            ungetc(c, input_file);
        }
        else if (stop_on_space && (c == ' '))
        {
            done = true;
            ungetc(c, input_file);
        }
        else
        {
            buffer[i] = (char) c;
            i++;
            if (i < max_chars)
            {
                c = fgetc(input_file);
            }
        }
    }
    buffer[i] = '\0';

    bool ran_out = ((i > max_chars) || (c == EOF));
    return !ran_out;
}

static inline void SkipSpaces(FILE *input_file)
{
    int c = ' ';
    while (!feof(input_file) && (c == ' '))
    {
        c = fgetc(input_file);
    }
    if (c != ' ')
    {
        ungetc(c, input_file);
    }
}

static inline bool ParseHeader(FILE *input_file, char *key, char *value)
{
    bool ok = ReadUntilDelim(input_file, key, MAX_HEADER_KEY_LEN, ':', true);
    if (ok)
    {
        SkipSpaces(input_file);
        ok = (fgetc(input_file) == ':');
        SkipSpaces(input_file);
        ok = ReadUntilDelim(input_file, value, MAX_HEADER_VAL_LEN, '\n', false);
    }
    ok = (fgetc(input_file) == '\n');
    return ok;
}

static bool ParseHeaders(FILE *input_file, RSA *privkey, size_t *enc_key_pos, size_t *n_enc_keys)
{
    assert(enc_key_pos != NULL);
    assert(n_enc_keys != NULL);

    /* Make sure these are always set by this function. */
    *enc_key_pos = 0;
    *n_enc_keys = 0;

    /* Actually works for a private RSA key too because it contains the public
     * key. */
    char *key_digest = GetPubkeyDigest(privkey);
    bool version_specified = false;
    bool found_matching_digest = false;
    size_t n_enc_for_headers = 0;

    char key[MAX_HEADER_KEY_LEN + 1];
    char value[MAX_HEADER_VAL_LEN + 1];

    while (ParseHeader(input_file, key, value))
    {
        Log(LOG_LEVEL_DEBUG, "Parsed header '%s: %s'", key, value);
        if (!CheckHeader(key, value))
        {
            free(key_digest);
            return false;
        }

        if (StringEqual(key, "Version"))
        {
            version_specified = true;
        }
        else if (StringEqual(key, "Encrypted-for"))
        {
            Log(LOG_LEVEL_DEBUG, "Encrypted for '%s'", value);
            if (StringEqual(value, key_digest))
            {
                found_matching_digest = true;
                *enc_key_pos = n_enc_for_headers;
            }
            n_enc_for_headers++;
        }

        /* headers are supposed to be terminated by a blank line */
        int next = fgetc(input_file);
        if (next == '\n')
        {
            if (!version_specified)
            {
                Log(LOG_LEVEL_ERR, "File format version not specified");
            }
            if (!found_matching_digest)
            {
                Log(LOG_LEVEL_ERR, "File not encrypted for host '%s'", key_digest);
            }
            *n_enc_keys = n_enc_for_headers;
            free(key_digest);
            return (version_specified && found_matching_digest);
        }
        else if (next == EOF)
        {
            Log(LOG_LEVEL_ERR, "Failed to parse headers from");
            free(key_digest);
            return false;
        }
        else
        {
            /* keep trying */
            ungetc(next, input_file);
        }
    }
    Log(LOG_LEVEL_ERR, "Failed to parse headers");
    free(key_digest);
    return false;
}

static bool RSADecrypt(RSA *privkey, const char *input_path, const char *output_path)
{
    FILE *input_file = OpenInputOutput(input_path, "r");
    if (input_file == NULL)
    {
        Log(LOG_LEVEL_ERR, "Cannot open input file '%s'", input_path);
        return false;
    }

    FILE *output_file = OpenInputOutput(output_path, "w");
    if (output_file == NULL)
    {
        Log(LOG_LEVEL_ERR, "Cannot open output file '%s'", output_path);
        fclose(input_file);
        return false;
    }

    EVP_PKEY *evp_key = EVP_PKEY_new();
    if (EVP_PKEY_set1_RSA(evp_key, privkey) == 0)
    {
        Log(LOG_LEVEL_ERR, "Failed to initialize decryption context");
        fclose(input_file);
        fclose(output_file);
        return false;
    }

    bool success = true;

    Log(LOG_LEVEL_VERBOSE, "Parsing headers");
    size_t our_key_pos;
    size_t n_enc_keys;
    if (!ParseHeaders(input_file, privkey, &our_key_pos, &n_enc_keys))
    {
        fclose(input_file);
        fclose(output_file);
        return false;
    }
    Log(LOG_LEVEL_DEBUG, "Parsed %zu keys", n_enc_keys);

    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    const int iv_size = EVP_CIPHER_iv_length(cipher);
    unsigned char iv[iv_size];

    const int key_size = EVP_PKEY_size(evp_key);
    unsigned char ek[key_size];
    unsigned char dev_null[key_size];

    const int block_size = EVP_CIPHER_block_size(cipher);

    char plaintext[block_size], ciphertext[2 * block_size];
    int pt_len;

    ssize_t n_read = ReadFileStreamToBuffer(input_file, iv_size, iv);
    if (n_read != iv_size)
    {
        Log(LOG_LEVEL_ERR, "Failed to read the IV from '%s'", input_path);
        goto cleanup;
    }

    /* just skip the keys that are not ours */
    size_t nth_key = 0;
    for (; nth_key < our_key_pos; nth_key++)
    {
        n_read = ReadFileStreamToBuffer(input_file, key_size, dev_null);
        if (n_read != key_size)
        {
            Log(LOG_LEVEL_ERR, "Failed to read the key from '%s'", input_path);
            goto cleanup;
        }
        Log(LOG_LEVEL_DEBUG, "Skipping key");
    }
    /* read our key */
    Log(LOG_LEVEL_DEBUG, "Reading key");
    n_read = ReadFileStreamToBuffer(input_file, key_size, ek);
    if (n_read != key_size)
    {
        Log(LOG_LEVEL_ERR, "Failed to read the key from '%s'", input_path);
        goto cleanup;
    }
    nth_key++;
    /* skip the remaining keys */
    for (; nth_key < n_enc_keys; nth_key++)
    {
        n_read = ReadFileStreamToBuffer(input_file, key_size, dev_null);
        if (n_read != key_size)
        {
            Log(LOG_LEVEL_ERR, "Failed to read the key from '%s'", input_path);
            goto cleanup;
        }
        Log(LOG_LEVEL_DEBUG, "Skipping key");
    }

    int ret = EVP_OpenInit(ctx, cipher, ek, key_size, iv, evp_key);
    if (ret == 0)
    {
        char *key_digest = GetPubkeyDigest(privkey);
        Log(LOG_LEVEL_ERR, "Failed to decrypt contents using key '%s'", key_digest);
        free(key_digest);
        success = false;
        goto cleanup;
    }

    Log(LOG_LEVEL_VERBOSE, "Decrypting data");
    size_t processed = 0;
    ssize_t n_written;
    while (success && !feof(input_file))
    {
        n_read = ReadFileStreamToBuffer(input_file, block_size, ciphertext);
        if (n_read == FILE_ERROR_READ)
        {
            Log(LOG_LEVEL_ERR, "Could not read file '%s'", input_path);
            success = false;
            break;
        }
        ret = EVP_OpenUpdate(ctx, plaintext, &pt_len, ciphertext, n_read);
        if (ret == 0)
        {
            Log(LOG_LEVEL_ERR, "Failed to decrypt data: %s",
                ERR_error_string(ERR_get_error(), NULL));
            success = false;
            break;
        }
        n_written = FullWrite(fileno(output_file), plaintext, pt_len);
        if (n_written < 0)
        {
            Log(LOG_LEVEL_ERR, "Could not write file '%s'", output_path);
            success = false;
            break;
        }
        processed += n_read;
        Log(LOG_LEVEL_VERBOSE, "%zu bytes processed", processed);
    }
    Log(LOG_LEVEL_VERBOSE, "Finalizing");
    ret = EVP_OpenFinal(ctx, plaintext, &pt_len);
    if (ret == 0)
    {
        Log(LOG_LEVEL_ERR, "Failed to decrypt data: %s",
            ERR_error_string(ERR_get_error(), NULL));
        success = false;
    }
    if (pt_len > 0)
    {
        n_written = FullWrite(fileno(output_file), plaintext, pt_len);
        if (n_written < 0)
        {
            Log(LOG_LEVEL_ERR, "Could not write file '%s'", output_path);
            success = false;
        }
    }
    OPENSSL_cleanse(plaintext, block_size);

  cleanup:
    fclose(input_file);
    fclose(output_file);
    EVP_PKEY_free(evp_key);
    EVP_CIPHER_CTX_free(ctx);
    return success;
}

static Seq *LoadPublicKeys(Seq *key_paths)
{
    const size_t n_keys = SeqLength(key_paths);
    Seq *pub_keys = SeqNew(n_keys, RSA_free);
    for (size_t i = 0; i < n_keys; i++)
    {
        const char *key_path = SeqAt(key_paths, i);
        Log(LOG_LEVEL_VERBOSE, "Reading key '%s'", key_path);
        RSA *pubkey = ReadPublicKey(key_path);
        if (pubkey == NULL)
        {
            SeqDestroy(pub_keys);
            return NULL;
        }
        SeqAppend(pub_keys, pubkey);
    }
    return pub_keys;
}

static void CFKeyCryptHelp()
{
    Writer *w = FileWriter(stdout);
    WriterWriteHelp(w, "cf-secret", OPTIONS, HINTS, COMMANDS, true, true);
    FileWriterDetach(w);
}

void CFKeyCryptMan()
{
    Writer *out = FileWriter(stdout);
    ManPageWrite(out, "cf-secret", time(NULL),
                 CF_SECRET_SHORT_DESCRIPTION,
                 CF_SECRET_MANPAGE_LONG_DESCRIPTION,
                 OPTIONS, HINTS, COMMANDS, true, true);
    FileWriterDetach(out);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        CFKeyCryptHelp();
        DoCleanupAndExit(EXIT_FAILURE);
    }

    opterr = 0;
    char *key_path_arg = NULL;
    char *input_path = NULL;
    char *output_path = NULL;
    char *host_arg = NULL;
    bool encrypt = false;
    bool decrypt = false;
    bool print_headers = false;

    size_t offset = 0;
    if (StringEqual(argv[1], "encrypt"))
    {
        encrypt = true;
        offset++;
    }
    else if (StringEqual(argv[1], "decrypt"))
    {
        offset++;
        decrypt = true;
    }
    else if (StringEqual(argv[1], "print-headers"))
    {
        print_headers = true;
        offset++;
    }

    int c = 0;
    while ((c = getopt_long(argc - offset, argv + offset, "hMedk:o:H:", OPTIONS, NULL)) != -1)
    {
        switch (c)
        {
            case 'h':
                CFKeyCryptHelp();
                DoCleanupAndExit(EXIT_SUCCESS);
                break;
            case 'M':
                CFKeyCryptMan();
                DoCleanupAndExit(EXIT_SUCCESS);
                break;
            case 'd':
                LogSetGlobalLevel(LOG_LEVEL_DEBUG);
                Log(LOG_LEVEL_DEBUG, "Debug log level enabled");
                break;
            case 'v':
                LogSetGlobalLevel(LOG_LEVEL_VERBOSE);
                Log(LOG_LEVEL_VERBOSE, "Verbose log level enabled");
                break;
            case 'I':
                LogSetGlobalLevel(LOG_LEVEL_INFO);
                Log(LOG_LEVEL_INFO, "Inform log level enabled");
                break;
            case 'g':
                LogSetGlobalLevelArgOrExit(optarg);
                break;
            case 'k':
                key_path_arg = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'H':
                host_arg = optarg;
                break;
            default:
                Log(LOG_LEVEL_ERR, "Unknown option '-%c'", optopt);
                CFKeyCryptHelp();
                DoCleanupAndExit(EXIT_FAILURE);
        }
    }

    if (!(decrypt || encrypt || print_headers))
    {
        printf("Command required. Specify either 'encrypt', 'decrypt' or 'print-headers'\n");
        CFKeyCryptHelp();
        DoCleanupAndExit(EXIT_FAILURE);
    }

    /* Increment 'optind' because of command being argv[0]. */
    optind++;
    input_path = argv[optind];

    /* Some more unexpected arguments? */
    if ((size_t) argc > (optind + offset))
    {
        Log(LOG_LEVEL_ERR, "Unexpected non-option argument: '%s'", argv[optind + 1]);
        DoCleanupAndExit(EXIT_FAILURE);
    }

    if (print_headers)
    {
        FILE *input_file = OpenInputOutput(input_path, "r");
        char key[MAX_HEADER_KEY_LEN + 1];
        char value[MAX_HEADER_VAL_LEN + 1];

        bool done = false;
        while (!done && ParseHeader(input_file, key, value))
        {
            Log(LOG_LEVEL_DEBUG, "Parsed header '%s: %s'", key, value);
            if (!CheckHeader(key, value))
            {
                fclose(input_file);
                DoCleanupAndExit(EXIT_FAILURE);
            }
            printf("%s: %s\n", key, value);

            /* headers are supposed to be terminated by a blank line */
            int next = fgetc(input_file);
            if (next == '\n')
            {
                done = true;
            }
            else
            {
                ungetc(next, input_file);
            }
        }
        fclose(input_file);
        DoCleanupAndExit(EXIT_SUCCESS);
    }

    if (decrypt && (host_arg == NULL) && (key_path_arg == NULL))
    {
        /* Decryption requires a private key which is usually only available for
         * the local host. Let's just default to localhost if no other specific
         * host/key is given for decryption. */
        Log(LOG_LEVEL_VERBOSE, "Using the localhost private key for decryption");
        host_arg = "localhost";
    }

    if ((host_arg != NULL) && (key_path_arg != NULL))
    {
        Log(LOG_LEVEL_ERR,
            "--host/-H is used to specify a public key and cannot be used with --key/-k");
        DoCleanupAndExit(EXIT_FAILURE);
    }

    if (input_path == NULL)
    {
        Log(LOG_LEVEL_ERR, "No input file specified (Use -h for help)");
        DoCleanupAndExit(EXIT_FAILURE);
    }
    if (output_path == NULL)
    {
        Log(LOG_LEVEL_ERR, "No output file specified (Use -h for help)");
        DoCleanupAndExit(EXIT_FAILURE);
    }

    CryptoInitialize();
    GenericAgentSetDefaultDigest(&CF_DEFAULT_DIGEST, &CF_DEFAULT_DIGEST_LEN);

    Seq *key_paths;
    if (key_path_arg != NULL)
    {
        Log(LOG_LEVEL_DEBUG, "-k/--key given: '%s'", key_path_arg);
        key_paths = SeqStringFromString(key_path_arg, ',');
    }
    else
    {
        key_paths = SeqNew(16, free);
    }

    // Default to localhost on encryption
    char *localhost = "127.0.0.1";
    if (encrypt && key_path_arg == NULL && host_arg == NULL)
    {
        host_arg = localhost;
    }

    if (host_arg != NULL)
    {
        Log(LOG_LEVEL_DEBUG, "-H/--host given: '%s'", host_arg);
        Seq *hosts = SeqStringFromString(host_arg, ',');
        const size_t n_hosts = SeqLength(hosts);
        for (size_t i = 0; i < n_hosts; i++)
        {
            HostRSAKeyType key_type = encrypt ? HOST_RSA_KEY_PUBLIC : HOST_RSA_KEY_PRIVATE;
            char *host = SeqAt(hosts, i);
            char *host_key_path = GetHostRSAKey(host, key_type);
            if (!host_key_path)
            {
                Log(LOG_LEVEL_ERR, "Unable to locate key for host '%s'", host);
                SeqDestroy(hosts);
                SeqDestroy(key_paths);
                DoCleanupAndExit(EXIT_FAILURE);
            }
            SeqAppend(key_paths, host_key_path);
        }
        SeqDestroy(hosts);
    }
    assert ((key_paths != NULL) && (SeqLength(key_paths) > 0));

    // Encrypt or decrypt
    bool success;
    if (encrypt)
    {
        Log(LOG_LEVEL_DEBUG, "Encrypting");
        Seq *pub_keys = LoadPublicKeys(key_paths);
        SeqDestroy(key_paths);
        if (pub_keys == NULL)
        {
            Log(LOG_LEVEL_ERR, "Failed to load public key(s)");
            DoCleanupAndExit(EXIT_FAILURE);
        }

        success = RSAEncrypt(pub_keys, input_path, output_path);
        SeqDestroy(pub_keys);
        if (!success)
        {
            Log(LOG_LEVEL_ERR, "Encryption failed");
            DoCleanupAndExit(EXIT_FAILURE);
        }
    }
    else if (decrypt)
    {
        Log(LOG_LEVEL_DEBUG, "Decrypting");
        const size_t n_keys = SeqLength(key_paths);
        if (n_keys > 1)
        {
            Log(LOG_LEVEL_ERR, "--decrypt requires only one key/host to be specified");
            SeqDestroy(key_paths);
            DoCleanupAndExit(EXIT_FAILURE);
        }
        RSA *private_key = ReadPrivateKey((char *) SeqAt(key_paths, 0));
        SeqDestroy(key_paths);
        success = RSADecrypt(private_key, input_path, output_path);
        RSA_free(private_key);
        if (!success)
        {
            Log(LOG_LEVEL_ERR, "Decryption failed");
            DoCleanupAndExit(EXIT_FAILURE);
        }
    }
    else
    {
        ProgrammingError("Unexpected error in cf-secret");
        DoCleanupAndExit(EXIT_FAILURE);
    }

    CallCleanupFunctions();
    return 0;
}
