resource "tls_private_key" "server_key" {
  algorithm = "RSA"
}

resource "local_file" "cloudmanthan_ca_key" {
  content  = tls_private_key.server_key.private_key_pem
  filename = "${path.module}/server.key"
}

resource "tls_self_signed_cert" "cm_ca_cert" {
  private_key_pem = tls_private_key.server_key.private_key_pem

  is_ca_certificate = true

  subject {
    country             = "IN"
    province            = "Mahrashatra"
    locality            = "Mumbai"
    common_name         = "Cloud Manthan Root CA"
    organization        = "Cloud Manthan Software Solutions Pvt Ltd."
    organizational_unit = "Cloud Manthan Root Certification Auhtority"
  }

  validity_period_hours = 43800 //  1825 days or 5 years

  allowed_uses = [
    "digital_signature",
    "cert_signing",
    "crl_signing",
  ]
}

resource "local_file" "cloudmanthan_ca_cert" {
  content  = tls_self_signed_cert.cm_ca_cert.cert_pem
  filename = "${path.module}/server.cer"
}