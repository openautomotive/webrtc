/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_RTC_BASE_FAKESSLIDENTITY_H_
#define WEBRTC_RTC_BASE_FAKESSLIDENTITY_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "webrtc/rtc_base/sslidentity.h"

namespace rtc {

class FakeSSLCertificate : public rtc::SSLCertificate {
 public:
  // SHA-1 is the default digest algorithm because it is available in all build
  // configurations used for unit testing.
  explicit FakeSSLCertificate(const std::string& data);
  explicit FakeSSLCertificate(const std::vector<std::string>& certs);
  FakeSSLCertificate(const FakeSSLCertificate&);
  ~FakeSSLCertificate() override;

  FakeSSLCertificate* GetReference() const override;
  std::string ToPEMString() const override;
  void ToDER(Buffer* der_buffer) const override;
  int64_t CertificateExpirationTime() const override;
  void SetCertificateExpirationTime(int64_t expiration_time);
  void set_digest_algorithm(const std::string& algorithm);
  bool GetSignatureDigestAlgorithm(std::string* algorithm) const override;
  bool ComputeDigest(const std::string& algorithm,
                     unsigned char* digest,
                     size_t size,
                     size_t* length) const override;
  std::unique_ptr<SSLCertChain> GetChain() const override;

 private:
  static FakeSSLCertificate* DupCert(FakeSSLCertificate cert) {
    return cert.GetReference();
  }
  static void DeleteCert(SSLCertificate* cert) { delete cert; }
  std::string data_;
  std::vector<FakeSSLCertificate> certs_;
  std::string digest_algorithm_;
  // Expiration time in seconds relative to epoch, 1970-01-01T00:00:00Z (UTC).
  int64_t expiration_time_;
};

class FakeSSLIdentity : public rtc::SSLIdentity {
 public:
  explicit FakeSSLIdentity(const std::string& data);
  explicit FakeSSLIdentity(const FakeSSLCertificate& cert);

  FakeSSLIdentity* GetReference() const override;
  const FakeSSLCertificate& certificate() const override;
  std::string PrivateKeyToPEMString() const override;
  std::string PublicKeyToPEMString() const override;
  virtual bool operator==(const SSLIdentity& other) const;

 private:
  FakeSSLCertificate cert_;
};

}  // namespace rtc

#endif  // WEBRTC_RTC_BASE_FAKESSLIDENTITY_H_
