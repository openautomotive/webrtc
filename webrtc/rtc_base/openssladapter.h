/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_RTC_BASE_OPENSSLADAPTER_H_
#define WEBRTC_RTC_BASE_OPENSSLADAPTER_H_

#include <map>
#include <string>
#include "webrtc/rtc_base/buffer.h"
#include "webrtc/rtc_base/messagehandler.h"
#include "webrtc/rtc_base/messagequeue.h"
#include "webrtc/rtc_base/ssladapter.h"

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct x509_store_ctx_st X509_STORE_CTX;
typedef struct ssl_session_st SSL_SESSION;

namespace rtc {

class SSLSessionCache {
 public:
  SSLSessionCache();
  ~SSLSessionCache();
  bool Lookup(const std::string& hostname, void** session) {
    auto it = sessions_.find(hostname);
    if (it == sessions_.end()) {
      return false;
    }
    *session = it->second;
    return true;
  }
  void Set(const std::string& hostname, void* session) {
    // TODO: Figure out what happens if there's already a value for the key
    sessions_[hostname] = session;
  }
 private:
  std::map<std::string, void*> sessions_;
};

///////////////////////////////////////////////////////////////////////////////

class OpenSSLAdapter : public SSLAdapter, public MessageHandler {
 public:
  static bool InitializeSSL(VerificationCallback callback);
  static bool InitializeSSLThread();
  static bool CleanupSSL();

  OpenSSLAdapter(AsyncSocket* socket);
  ~OpenSSLAdapter() override;

  void set_session_cache(SSLSessionCache* cache) { ssl_session_cache_ = cache; }

  void SetMode(SSLMode mode) override;
  int StartSSL(const char* hostname, bool restartable) override;
  int Send(const void* pv, size_t cb) override;
  int SendTo(const void* pv, size_t cb, const SocketAddress& addr) override;
  int Recv(void* pv, size_t cb, int64_t* timestamp) override;
  int RecvFrom(void* pv,
               size_t cb,
               SocketAddress* paddr,
               int64_t* timestamp) override;
  int Close() override;

  // Note that the socket returns ST_CONNECTING while SSL is being negotiated.
  ConnState GetState() const override;

protected:
 void OnConnectEvent(AsyncSocket* socket) override;
 void OnReadEvent(AsyncSocket* socket) override;
 void OnWriteEvent(AsyncSocket* socket) override;
 void OnCloseEvent(AsyncSocket* socket, int err) override;

private:
  enum SSLState {
    SSL_NONE, SSL_WAIT, SSL_CONNECTING, SSL_CONNECTED, SSL_ERROR
  };

  enum { MSG_TIMEOUT };

  int BeginSSL();
  int ContinueSSL();
  void Error(const char* context, int err, bool signal = true);
  void Cleanup();

  // Return value and arguments have the same meanings as for Send; |error| is
  // an output parameter filled with the result of SSL_get_error.
  int DoSslWrite(const void* pv, size_t cb, int* error);

  void OnMessage(Message* msg) override;

  static bool VerifyServerName(SSL* ssl, const char* host,
                               bool ignore_bad_cert);
  bool SSLPostConnectionCheck(SSL* ssl, const char* host);
#if !defined(NDEBUG)
  static void SSLInfoCallback(const SSL* ssl, int where, int ret);
#endif
  static int SSLVerifyCallback(int ok, X509_STORE_CTX* store);
  static VerificationCallback custom_verify_callback_;
  friend class OpenSSLStreamAdapter;  // for custom_verify_callback_;
  static int NewSSLSessionCallback(SSL* ssl, SSL_SESSION* session);

  static bool ConfigureTrustedRootCertificates(SSL_CTX* ctx);
  SSL_CTX* SetupSSLContext();

  SSLState state_;
  bool ssl_read_needs_write_;
  bool ssl_write_needs_read_;
  // If true, socket will retain SSL configuration after Close.
  bool restartable_;

  // This buffer is used if SSL_write fails with SSL_ERROR_WANT_WRITE, which
  // means we need to keep retrying with *the same exact data* until it
  // succeeds. Afterwards it will be cleared.
  Buffer pending_data_;

  SSL* ssl_;
  SSL_CTX* ssl_ctx_;
  std::string ssl_host_name_;
  // Do DTLS or not
  SSLMode ssl_mode_;
  SSLSessionCache* ssl_session_cache_;

  bool custom_verification_succeeded_;
};

/////////////////////////////////////////////////////////////////////////////

} // namespace rtc


#endif // WEBRTC_RTC_BASE_OPENSSLADAPTER_H_
