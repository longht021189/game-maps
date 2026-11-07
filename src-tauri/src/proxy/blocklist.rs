//! Ad and tracking requests, answered locally instead of reaching the network.
//!
//! - [`DOMAINS`]: a request is blocked when its host is the domain or one of
//!   its subdomains (`doubleclick.net` blocks `cm.g.doubleclick.net`).
//! - [`PATHS`]: same host matching, but only for paths starting with the prefix,
//!   for sites that also serve things the page needs (e.g. reCAPTCHA on
//!   `www.google.com`).

use tauri::http::Uri;

/// Blocked domains, including their subdomains.
const DOMAINS: &[&str] = &[
    // Google ads
    "doubleclick.net",
    "googlesyndication.com",
    "googleadservices.com",
    "googletagmanager.com",
    "googletagservices.com",
    "adtrafficquality.google",
    "fundingchoicesmessages.google.com",
    "2mdn.net",
    "ampproject.org",
    // Ziff Davis (mapgenie's publisher) ad stack
    "ziffstatic.com",
    "zdbb.net",
    "ignimgs.com",
    // Ad networks, header bidding, user-sync
    "33across.com",
    "360yield.com",
    "1rx.io",
    "3lift.com",
    "ad-m.asia",
    "adform.net",
    "admanmedia.com",
    "adnxs.com",
    "adsrvr.org",
    "adx.opera.com",
    "amazon-adsystem.com",
    "bidpapers.com",
    "bidr.io",
    "bidswitch.net",
    "blismedia.com",
    "casalemedia.com",
    "cinarra.com",
    "contextweb.com",
    "cootlogix.com",
    "cpmstar.com",
    "creativecdn.com",
    "criteo.com",
    "criteo.net",
    "deepintent.com",
    "demdex.net",
    "dotomi.com",
    "eu-1-id5-sync.com",
    "everesttech.net",
    "fastclick.net",
    "fwmrm.net",
    "gumgum.com",
    "id5-sync.com",
    "indexww.com",
    "ipredictive.com",
    "kargo.com",
    "kueezrtb.com",
    "ladsp.com",
    "liadm.com",
    "lijit.com",
    "loopme.me",
    "media.net",
    "mediagotechnology.com",
    "mfadsrvr.com",
    "nexx360.io",
    "nrich.ai",
    "onetag-sys.com",
    "openx.net",
    "outbrain.com",
    "pbs.yahoo.com",
    "pmbmonetize.live",
    "postrelease.com",
    "presage.io",
    "pubmatic.com",
    "px.ads.linkedin.com",
    "qvdt3feo.com",
    "rapidedge.io",
    "rbstsystems.live",
    "rfihub.com",
    "richaudience.com",
    "rubiconproject.com",
    "seedtag.com",
    "sitescout.com",
    "smaato.net",
    "smartadserver.com",
    "sonobi.com",
    "sportradarserving.com",
    "ssp.wp.pl",
    "stackadapt.com",
    "sundaysky.com",
    "sundayskyads.com",
    "syncingbridge.com",
    "tapad.com",
    "the-ozone-project.com",
    "trustedstack.com",
    "turn.com",
    "tynt.com",
    "unrulymedia.com",
    "yellowblue.io",
    "zucks.net",
    // Ad verification
    "confiant-integrations.net",
    "doubleverify.com",
    "dv.tech",
    // Analytics / tracking
    "analytics.google.com",
    "google-analytics.com",
    "analytics.yahoo.com",
    "bat.bing.com",
    "c.bing.com",
    "chartbeat.com",
    "chartbeat.net",
    "clarity.ms",
    "cloudflareinsights.com",
    "scorecardresearch.com",
    // Cookie consent (OneTrust)
    "cookielaw.org",
    "onetrust.com",
];

/// Blocked path prefixes on otherwise allowed domains (and their subdomains).
const PATHS: &[(&str, &[&str])] = &[
    (
        "google.com",
        &["/pagead/", "/ads/", "/rmkt/", "/ccm/", "/measurement/"],
    ),
    (
        "google.com.vn",
        &["/pagead/", "/ads/", "/rmkt/", "/ccm/", "/measurement/"],
    ),
];

/// Whether `uri` is an ad or tracking request.
pub fn is_blocked(uri: &Uri) -> bool {
    let Some(host) = uri.host() else {
        return false;
    };
    let host = host.to_ascii_lowercase();

    if DOMAINS.iter().any(|domain| is_same_or_subdomain(&host, domain)) {
        return true;
    }

    PATHS.iter().any(|(domain, prefixes)| {
        is_same_or_subdomain(&host, domain)
            && prefixes.iter().any(|prefix| uri.path().starts_with(prefix))
    })
}

/// `host` is `domain` or a subdomain of it (`a.b.com` for `b.com`, not `ab.com`).
fn is_same_or_subdomain(host: &str, domain: &str) -> bool {
    host.strip_suffix(domain)
        .is_some_and(|rest| rest.is_empty() || rest.ends_with('.'))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn blocked(uri: &str) -> bool {
        is_blocked(&uri.parse().unwrap())
    }

    #[test]
    fn blocks_domains_and_subdomains() {
        assert!(blocked("https://doubleclick.net/"));
        assert!(blocked("https://cm.g.doubleclick.net/pixel?x=1"));
        assert!(blocked("https://CDN.ZiffStatic.com/jst/zdconsent.js"));
        assert!(blocked("https://ups.analytics.yahoo.com/ups/58251/sync"));
        assert!(blocked("https://cdn.cookielaw.org/scripttemplates/otSDKStub.js"));
        assert!(blocked("https://geolocation.onetrust.com/cookieconsentpub/v1/geo/location"));
    }

    #[test]
    fn does_not_block_lookalikes_or_parents() {
        assert!(!blocked("https://notdoubleclick.net/"));
        assert!(!blocked("https://doubleclick.net.example.com/"));
        // Only `pbs.yahoo.com` / `analytics.yahoo.com` are blocked, not all of yahoo.
        assert!(!blocked("https://www.yahoo.com/"));
    }

    #[test]
    fn blocks_paths_only_where_listed() {
        assert!(blocked("https://www.google.com/pagead/1p-user-list/1/"));
        assert!(blocked("https://www.google.com.vn/ads/ga-audiences"));
        assert!(!blocked("https://www.google.com/recaptcha/api2/aframe"));
        assert!(!blocked("https://ajax.googleapis.com/ajax/libs/jquery/3.7.0/jquery.min.js"));
    }

    #[test]
    fn keeps_mapgenie_and_its_cdns() {
        assert!(!blocked("https://mapgenie.io/"));
        assert!(!blocked("https://cdn.mapgenie.io/images/x.png"));
        assert!(!blocked("https://cdnjs.cloudflare.com/ajax/libs/axios/1.20.0/axios.min.js"));
        assert!(!blocked("https://use.typekit.net/dhf1zwt.css"));
    }
}
