#include "network-ads.hpp"

std::vector<std::string> ads_domains = {
  "doubleclick.net",
  "fastclick.net",
  "googletagmanager.com",
  "ampproject.org",
  "scorecardresearch.com",
  "2mdn.net",
  "doubleverify.com",
  "ignimgs.com",
  "amazon-adsystem.com",
  "confiant-integrations.net",
  "jsdelivr.net",
  "ziffstatic.com",
  "adnxs.com",
  "chartbeat.net",
  "chartbeat.com",
  "zdbb.net",
  "liadm.com"
};

CORE_PRIVATE bool core_network_is_ads(const NetworkRequest* request) {
  if (std::ranges::find(ads_domains, request->m_parsed_url->domain) != ads_domains.end()) {
    return true;
  }
  return false;
}

CORE_PRIVATE const std::vector<std::string>& core_network_ads_domains() {
  return ads_domains;
}
