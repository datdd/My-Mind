#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>

#define CIPHER_BUF_SIZE 4096
#define AES_256_KEY_SIZE 32
#define AES_BLOCK_SIZE 16
#define ENCRYPT 1
#define DECRYPT 0

/* Key to use for encrpytion and decryption */
#ifdef AES_256_GEN_KEY
unsigned char key[AES_256_KEY_SIZE + 1] = AES_256_GEN_KEY;
#else
unsigned char key[AES_256_KEY_SIZE + 1] = "Z47HRKEbJHqDmkDJSEDtV7CnBc2KvLSP";
#endif

/* Initialization Vector */
#ifdef AES_256_GEN_IV
unsigned char iv[AES_BLOCK_SIZE + 1] = AES_256_GEN_IV;
#else
unsigned char iv[AES_BLOCK_SIZE + 1] = "SVca4E9qjDy9Yux9";
#endif

typedef struct _cipher_params_t {
    unsigned char *key;            /* Symmetric key to use */
    unsigned char *iv;             /* The IV to use */
    unsigned int   encrypt;        /* 1 Indicate that we want to encrypt */
    const EVP_CIPHER *cipher_type; /* EVP_CIPHER type */
} cipher_params_t;

void 
cleanup(EVP_CIPHER_CTX *ctx, void *buf)
{
   if (NULL != buf)
   {
      free(buf);
   }
   EVP_CIPHER_CTX_cleanup(ctx);
}

int aes_init_params(cipher_params_t *params, unsigned char *key,
   unsigned char *iv, unsigned int encrypt, const EVP_CIPHER *cipher_type)
{
   if (NULL == params || NULL == key || NULL == iv || NULL == cipher_type)
   {
      printf("ERROR: Invalid params\n");
      return -1;
   }

   params->key = key;
   params->iv = iv;
   params->encrypt = encrypt;
   params->cipher_type = cipher_type;

   return 0;
}

int
aes_init(EVP_CIPHER_CTX *ctx, cipher_params_t *params)
{
    printf("DEBUG %s():%u\n", __FUNCTION__, __LINE__);
   /* Don't set key or IV right away; we want to check lengths */
   if(!EVP_CipherInit_ex(ctx, params->cipher_type, NULL, NULL, NULL, params->encrypt)){
      printf("ERROR: EVP_CipherInit_ex failed. OpenSSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
      return -1;
   }

   OPENSSL_assert(EVP_CIPHER_CTX_key_length(ctx) == AES_256_KEY_SIZE);
   OPENSSL_assert(EVP_CIPHER_CTX_iv_length(ctx) == AES_BLOCK_SIZE);

   /* Now we can set key and IV */
   if(!EVP_CipherInit_ex(ctx, NULL, NULL, params->key, params->iv, params->encrypt)){
      printf("ERROR: EVP_CipherInit_ex failed. OpenSSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
      return -1;
   }

   return 0;
}

int
aes_256_file_encrypt_decrypt(int encrypt, FILE *ifp, FILE *ofp)
{
    EVP_CIPHER_CTX *ctx;
    const EVP_CIPHER *cipher_type;
    int cipher_block_size, cipher_len, num_bytes_read, out_len;
    unsigned char in_buf[CIPHER_BUF_SIZE], *out_buf;
    cipher_params_t params;

    ctx = EVP_CIPHER_CTX_new();
    if(ctx == NULL){
        printf("ERROR: EVP_CIPHER_CTX_new failed. OpenSSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }
    cipher_type = EVP_aes_256_cbc();
    if (aes_init_params(&params, key, iv, encrypt, cipher_type) != 0)
    {
        EVP_CIPHER_CTX_cleanup(ctx);
        return -1;
    }

    cipher_block_size = EVP_CIPHER_block_size(params.cipher_type);
    cipher_len = CIPHER_BUF_SIZE + cipher_block_size;
    out_buf = (unsigned char *)malloc(cipher_len);

    if (NULL == out_buf)
    {
        /* Unable to allocate memory on heap */
        printf( "ERROR: malloc error: %s\n", strerror(errno));
        EVP_CIPHER_CTX_cleanup(ctx);
        return -1;
    }

    if (aes_init(ctx, &params) != 0)
    {
        printf( "ERROR: EVP_CipherInit_ex failed. OpenSSL error\n");
        cleanup(ctx, out_buf);
        return -1;
    }

    memset(out_buf, 0x0, cipher_len);
    while (1)
    {
        num_bytes_read = fread(in_buf, sizeof(unsigned char), CIPHER_BUF_SIZE, ifp);
        if (ferror(ifp))
        {
            printf("ERROR: fread error: %s\n", strerror(errno));
            cleanup(ctx, out_buf);
        }

        if(!EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, num_bytes_read))
        {
            printf("ERROR: EVP_CipherUpdate failed. OpenSSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
            cleanup(ctx, out_buf);
            return -1;
        }

        fwrite(out_buf, sizeof(unsigned char), out_len, ofp);
        if (ferror(ofp)) {
            printf("ERROR: fwrite error: %s\n", strerror(errno));
            cleanup(ctx, out_buf);
            return -1;
        }

        if (num_bytes_read < CIPHER_BUF_SIZE)
        {
            break;
        }
    }
    if(!EVP_CipherFinal_ex(ctx, out_buf, &out_len)){
        fprintf(stderr, "ERROR: EVP_CipherFinal_ex failed. OpenSSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
        cleanup(ctx, out_buf);
        return -1;
    }

    fwrite(out_buf, sizeof(unsigned char), out_len, ofp);
    if (ferror(ofp)) {
        fprintf(stderr, "ERROR: fwrite error: %s\n", strerror(errno));
        cleanup(ctx, out_buf);
        return -1;
    }
    cleanup(ctx, out_buf);

    return 0;
}


int
vnptt_file_encode (int encrypt, const char *src, const char *dst)
{
    FILE *fp_inp = NULL;
    FILE *fp_out = NULL;

    if (NULL == src)
    {
        return -1;
    }

    fp_inp = fopen (src, "rb");
    if (NULL == fp_inp)
    {
        return -1;
    }

    fp_out = fopen (dst, "wb");
    if (NULL == fp_out)
    {
        fclose (fp_inp);
        return -1;
    }

#if 1
    printf("%s():%u src: %s, dst: %s\n", __FUNCTION__, __LINE__, src, dst);
    aes_256_file_encrypt_decrypt(encrypt, fp_inp, fp_out);
    
#else
    vnptt_file_encrypt_decrypt(fp_inp, fp_out);
#endif

    fclose (fp_inp);
    fclose (fp_out);

    return 0;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}


int main(int argc, char *argv[])
{
    vnptt_file_encode(0, "./bin/romfile.cfg", "./bin/romfile.decrypted");
    vnptt_file_encode(1, "./bin/romfile.decrypted", "./bin/romfile.encrypted");

    return 0;
}
