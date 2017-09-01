/*
 *  Copyright 2017 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/rtc_base/fakesslidentity.h"

#include <string>

#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/messagedigest.h"

namespace rtc {

FakeSSLCertificate::FakeSSLCertificate(const std::string& data)
    : data_(data), digest_algorithm_(DIGEST_SHA_1), expiration_time_(-1) {}

FakeSSLCertificate::FakeSSLCertificate(const std::vector<std::string>& certs)
    : data_(certs.front()),
      digest_algorithm_(DIGEST_SHA_1),
      expiration_time_(-1) {
  std::vector<std::string>::const_iterator it;
  // Skip certs[0].
  for (it = certs.begin() + 1; it != certs.end(); ++it) {
    certs_.push_back(FakeSSLCertificate(*it));
  }
}

FakeSSLCertificate::FakeSSLCertificate(const FakeSSLCertificate&) = default;

FakeSSLCertificate::~FakeSSLCertificate() = default;

FakeSSLCertificate* FakeSSLCertificate::GetReference() const {
  return new FakeSSLCertificate(*this);
}

std::string FakeSSLCertificate::ToPEMString() const {
  return data_;
}

void FakeSSLCertificate::ToDER(Buffer* der_buffer) const {
  std::string der_string;
  RTC_CHECK(SSLIdentity::PemToDer(kPemTypeCertificate, data_, &der_string));
  der_buffer->SetData(der_string.c_str(), der_string.size());
}

int64_t FakeSSLCertificate::CertificateExpirationTime() const {
  return expiration_time_;
}

void FakeSSLCertificate::SetCertificateExpirationTime(int64_t expiration_time) {
  expiration_time_ = expiration_time;
}

void FakeSSLCertificate::set_digest_algorithm(const std::string& algorithm) {
  digest_algorithm_ = algorithm;
}

bool FakeSSLCertificate::GetSignatureDigestAlgorithm(
    std::string* algorithm) const {
  *algorithm = digest_algorithm_;
  return true;
}

bool FakeSSLCertificate::ComputeDigest(const std::string& algorithm,
                                       unsigned char* digest,
                                       size_t size,
                                       size_t* length) const {
  *length =
      rtc::ComputeDigest(algorithm, data_.c_str(), data_.size(), digest, size);
  return (*length != 0);
}

std::unique_ptr<SSLCertChain> FakeSSLCertificate::GetChain() const {
  if (certs_.empty())
    return nullptr;
  std::vector<SSLCertificate*> new_certs(certs_.size());
  std::transform(certs_.begin(), certs_.end(), new_certs.begin(), DupCert);
  std::unique_ptr<SSLCertChain> chain(new SSLCertChain(new_certs));
  std::for_each(new_certs.begin(), new_certs.end(), DeleteCert);
  return chain;
}

FakeSSLIdentity::FakeSSLIdentity(const std::string& data) : cert_(data) {}

FakeSSLIdentity::FakeSSLIdentity(const FakeSSLCertificate& cert)
    : cert_(cert) {}

FakeSSLIdentity* FakeSSLIdentity::GetReference() const {
  return new FakeSSLIdentity(*this);
}

const FakeSSLCertificate& FakeSSLIdentity::certificate() const {
  return cert_;
}
std::string FakeSSLIdentity::PrivateKeyToPEMString() const {
  RTC_NOTREACHED();  // Not implemented.
  return "";
}

std::string FakeSSLIdentity::PublicKeyToPEMString() const {
  RTC_NOTREACHED();  // Not implemented.
  return "";
}

bool FakeSSLIdentity::operator==(const SSLIdentity& other) const {
  RTC_NOTREACHED();  // Not implemented.
  return false;
}

}  // namespace rtc
