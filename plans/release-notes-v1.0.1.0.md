# Alter Ego v1.0.1.0

A resilience and bandwidth pass, plus a small Clears-tab addition.

## What's New

- **Raid wing rotation chips.** The Clears tab now flags whichever wing is *Emboldened* and which has *Call of the Mists* each week, with the official wiki icons and a tooltip explaining the buff. Rotates Monday 08:30 UTC.

## Reduced API traffic

The addon now hits the GW2 API and the wiki far less often, and recovers gracefully when either is down or rate-limiting:

- **Persistent reference caches.** Items, skins, specializations, traits, skills, itemstats, and dye colours are now cached to disk between sessions — once a character has been refreshed, full equipment and build display works offline.
- **Build name back-fill from cache.** Saved builds keep their original AE2 code and resolve missing rune/relic/sigil/stat names from cache when the API is unavailable.
- **Icon loading uses on-disk cache before requiring a URL** — cached icons now appear even when item info hasn't loaded yet.
- **Persistent negative wiki cache.** Failed skin-image and currency-icon lookups are remembered for 7 days, so the 4–6-request wiki chain no longer re-runs every session for skins with no wiki page.
- **Commerce price cache survives restarts.** Trading-post prices are saved to disk and dropped after 15 minutes.
- **Daily achievements cache.** The daily-achievements endpoint is hit at most once per hour.
- **Batched wiki currency lookups.** Up to 50 currency icons are resolved in a single wiki API call instead of one per icon.
- **Atomic file writes.** Cache files are written via a temp-file rename so a crash mid-write can no longer corrupt them.

## Install

Drop `AlterEgo.dll` into `<GW2>\addons\` and restart Guild Wars 2 (or use Nexus's hot-reload).
