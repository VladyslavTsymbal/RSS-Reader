#include <stdio.h>
#include <stdlib.h>
#include <openssl/bio.h> /* BasicInput/Output streams */
#include <openssl/err.h> /* errors */
#include <openssl/ssl.h> /* core library */
#include <string>

constexpr size_t BUFF_SIZE = 1024;

void reportAndExit(const char* msg)
{
  perror(msg);
  ERR_print_errors_fp(stderr);
  exit(-1);
}

void initSSL()
{
  SSL_load_error_strings();
  SSL_library_init();
}

void cleanUpSSL(SSL_CTX* ctx, BIO* bio) {
  SSL_CTX_free(ctx);
  BIO_free_all(bio);
}

void secureConnect(const char* hostname) {
  char name[BUFF_SIZE];
  char request[BUFF_SIZE];
  char response[BUFF_SIZE];

  const SSL_METHOD* method = TLS_client_method();
  if (NULL == method) reportAndExit("TLS_client_method...");

  SSL_CTX* ctx = SSL_CTX_new(method);
  if (NULL == ctx) reportAndExit("SSL_CTX_new...");

  BIO* bio = BIO_new_ssl_connect(ctx);
  if (NULL == bio) reportAndExit("BIO_new_ssl_connect...");

  SSL* ssl = NULL;

  /* link bio channel, SSL session, and server endpoint */

  sprintf(name, "%s:%s", hostname, "https");
  BIO_get_ssl(bio, &ssl); /* session */
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY); /* robustness */
  BIO_set_conn_hostname(bio, name); /* prepare to connect */

  /* try to connect */
  if (BIO_do_connect(bio) <= 0) {
    cleanUpSSL(ctx, bio);
    reportAndExit("BIO_do_connect...");
  }

  /* verify truststore, check cert */
  if (!SSL_CTX_load_verify_locations(ctx,
                                      "/etc/ssl/certs/ca-certificates.crt", /* truststore */
                                      "/etc/ssl/certs/")) /* more truststore */
    reportAndExit("SSL_CTX_load_verify_locations...");

  long verify_flag = SSL_get_verify_result(ssl);
  if (verify_flag != X509_V_OK)
    fprintf(stderr,
            "##### Certificate verification error (%i) but continuing...\n",
            (int) verify_flag);

  /* now fetch sample data */
  sprintf(request,
          "GET /feed/ HTTP/1.1\x0D\x0AHost: %s\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A",
          hostname);
  BIO_puts(bio, request);

  /* read HTTP response from server and print to stdout */
  while (true) {
    memset(response, '\0', sizeof(response));
    int n = BIO_read(bio, response, BUFF_SIZE);

    if (n <= 0) break; /* 0 is end-of-stream, < 0 is an error */
    puts(response);
  }

  cleanUpSSL(ctx, bio);
}

int main() {
  initSSL();

  const char* hostname = "techcrunch.com";
  fprintf(stderr, "Trying an HTTPS connection to %s...\n", hostname);
  secureConnect(hostname);

  return 0;
}
