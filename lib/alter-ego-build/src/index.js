/**
 * @module alter-ego-build
 * Encode and decode Alter Ego AE2 build codes for Guild Wars 2.
 * Zero dependencies. Works in browser and Node.js.
 *
 * @example
 * import { encodeAE2, decodeAE2 } from 'alter-ego-build';
 *
 * const code = encodeAE2({
 *   chatLink: '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]',
 *   gameMode: 'Raid',
 *   gear: {
 *     Helm: { statId: 1077 },
 *     WeaponA1: { statId: 1077, sigilId: 24615, weaponType: 'Greatsword', sigil2Id: 24868 },
 *   },
 *   runeId: 24836,
 *   relicId: 100916,
 * });
 *
 * const build = decodeAE2(code);
 */

// ── Constants ──────────────────────────────────────────────────────────

const AE2_VERSION = 3;

const SLOT_NAMES = [
  'Helm', 'Shoulders', 'Coat', 'Gloves', 'Leggings', 'Boots',
  'WeaponA1', 'WeaponA2', 'WeaponB1', 'WeaponB2',
  'Backpack', 'Accessory1', 'Accessory2', 'Amulet', 'Ring1', 'Ring2',
];

const SLOT_INDEX = Object.fromEntries(SLOT_NAMES.map((n, i) => [n, i]));

const WEAPON_SLOTS = new Set(['WeaponA1', 'WeaponA2', 'WeaponB1', 'WeaponB2']);
const WEAPON_SLOT_START = 6;
const WEAPON_SLOT_END = 9;

const GAME_MODES = ['PvE', 'WvW', 'PvP', 'Raid', 'Fractal', 'Other'];
const GAME_MODE_INDEX = Object.fromEntries(GAME_MODES.map((m, i) => [m, i]));

const WEAPON_TYPE_TO_ID = {
  Axe: 5, Longbow: 35, Dagger: 47, Focus: 49, Greatsword: 50,
  Hammer: 51, Mace: 53, Pistol: 54, Rifle: 85, Scepter: 86,
  Shield: 87, Staff: 89, Sword: 90, Torch: 102, Warhorn: 103,
  Shortbow: 107, Spear: 265,
};

const WEAPON_ID_TO_TYPE = Object.fromEntries(
  Object.entries(WEAPON_TYPE_TO_ID).map(([k, v]) => [v, k])
);

// ── Base64 ─────────────────────────────────────────────────────────────

const B64 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
const B64_LOOKUP = new Uint8Array(128);
for (let i = 0; i < B64.length; i++) B64_LOOKUP[B64.charCodeAt(i)] = i;

function b64Encode(bytes) {
  let out = '';
  for (let i = 0; i < bytes.length; i += 3) {
    const b0 = bytes[i], b1 = bytes[i + 1] ?? 0, b2 = bytes[i + 2] ?? 0;
    out += B64[(b0 >> 2) & 0x3F];
    out += B64[((b0 << 4) | (b1 >> 4)) & 0x3F];
    out += (i + 1 < bytes.length) ? B64[((b1 << 2) | (b2 >> 6)) & 0x3F] : '=';
    out += (i + 2 < bytes.length) ? B64[b2 & 0x3F] : '=';
  }
  return out;
}

function b64Decode(str) {
  str = str.replace(/=+$/, '');
  const out = [];
  for (let i = 0; i < str.length; i += 4) {
    const a = B64_LOOKUP[str.charCodeAt(i)];
    const b = B64_LOOKUP[str.charCodeAt(i + 1)] ?? 0;
    const c = B64_LOOKUP[str.charCodeAt(i + 2)] ?? 0;
    const d = B64_LOOKUP[str.charCodeAt(i + 3)] ?? 0;
    out.push((a << 2) | (b >> 4));
    if (i + 2 < str.length) out.push(((b << 4) | (c >> 2)) & 0xFF);
    if (i + 3 < str.length) out.push(((c << 6) | d) & 0xFF);
  }
  return new Uint8Array(out);
}

// ── Binary helpers ─────────────────────────────────────────────────────

function pushU16(buf, v) { buf.push(v & 0xFF, (v >> 8) & 0xFF); }
function pushU32(buf, v) {
  buf.push(v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF, (v >>> 24) & 0xFF);
}
function readU16(data, off) { return data[off] | (data[off + 1] << 8); }
function readU32(data, off) {
  return (data[off] | (data[off + 1] << 8) | (data[off + 2] << 16) | (data[off + 3] << 24)) >>> 0;
}
function popcount16(v) {
  let c = 0;
  for (let i = 0; i < 16; i++) if (v & (1 << i)) c++;
  return c;
}

// ── Chat link helpers ──────────────────────────────────────────────────

function extractLinkPayload(chatLink) {
  const m = chatLink.match(/^\[&(.+?)]$/);
  if (!m) throw new Error('Invalid chat link format. Expected [&...] wrapper.');
  return m[1];
}

// ── encodeAE2 ──────────────────────────────────────────────────────────

/**
 * Encode a build into an AE2 code string.
 *
 * @param {Object} options
 * @param {string} options.chatLink - GW2 build template chat link, e.g. '[&DQE...]'
 * @param {string} [options.gameMode='PvE'] - One of: PvE, WvW, PvP, Raid, Fractal, Other
 * @param {Object.<string, GearSlot>} [options.gear] - Map of slot name to gear data
 * @param {number} [options.runeId] - GW2 item ID of the rune (0 or omit if none)
 * @param {number} [options.relicId] - GW2 exotic relic item ID (0 or omit if none)
 * @returns {string} AE2 code string, e.g. 'AE2:AwgD...'
 *
 * @typedef {Object} GearSlot
 * @property {number} statId - GW2 API itemstat ID
 * @property {number} [sigilId] - Sigil item ID (weapon slots only)
 * @property {string} [weaponType] - Weapon type name (weapon slots only)
 * @property {number} [sigil2Id] - Second sigil item ID (weapon slots only, for 2H weapons)
 */
export function encodeAE2({ chatLink, gameMode = 'PvE', gear, runeId, relicId }) {
  if (!chatLink) throw new Error('chatLink is required');

  const linkB64 = extractLinkPayload(chatLink);
  const linkBytes = b64Decode(linkB64);

  const hasGear = gear && Object.keys(gear).length > 0;
  const hasRune = (runeId || 0) > 0;
  const hasRelic = (relicId || 0) > 0;

  const modeIdx = GAME_MODE_INDEX[gameMode] ?? 0;
  let flags = modeIdx & 0x07;
  if (hasGear) flags |= 0x08;
  if (hasRune) flags |= 0x10;
  if (hasRelic) flags |= 0x20;

  const buf = [];

  // Header
  buf.push(AE2_VERSION);
  buf.push(flags);
  buf.push(linkBytes.length);
  for (let i = 0; i < linkBytes.length; i++) buf.push(linkBytes[i]);

  // Gear
  if (hasGear) {
    let gearMask = 0;
    for (const [slot, gs] of Object.entries(gear)) {
      const idx = SLOT_INDEX[slot];
      if (idx !== undefined && gs.statId) gearMask |= (1 << idx);
    }
    pushU16(buf, gearMask);

    // Stat IDs in bit order
    for (let i = 0; i < 16; i++) {
      if (!(gearMask & (1 << i))) continue;
      const slotName = SLOT_NAMES[i];
      const gs = gear[slotName];
      pushU16(buf, gs ? (gs.statId & 0xFFFF) : 0);
    }
  }

  // Rune
  if (hasRune) pushU32(buf, runeId);

  // Relic
  if (hasRelic) pushU32(buf, relicId);

  // Weapon slot data (sigil + weapon type + sigil2)
  if (hasGear) {
    for (let i = WEAPON_SLOT_START; i <= WEAPON_SLOT_END; i++) {
      const slotName = SLOT_NAMES[i];
      const gs = gear[slotName];
      if (!gs || !gs.statId) continue;
      pushU32(buf, gs.sigilId || 0);
      pushU16(buf, WEAPON_TYPE_TO_ID[gs.weaponType] || 0);
      pushU32(buf, gs.sigil2Id || 0);
    }
  }

  return 'AE2:' + b64Encode(new Uint8Array(buf));
}

// ── decodeAE2 ──────────────────────────────────────────────────────────

/**
 * Decode an AE2 code string into structured build data.
 *
 * @param {string} ae2code - AE2 code string, e.g. 'AE2:AwgD...'
 * @returns {Object} Decoded build data
 * @returns {string} .chatLink - Reconstructed GW2 chat link
 * @returns {string} .gameMode - Game mode name
 * @returns {Object.<string, Object>} .gear - Gear slot data
 * @returns {number} .runeId - Rune item ID (0 if none)
 * @returns {number} .relicId - Relic item ID (0 if none)
 * @returns {number} .version - AE2 format version
 */
export function decodeAE2(ae2code) {
  if (typeof ae2code !== 'string' || !ae2code.startsWith('AE2:')) {
    throw new Error('Not an AE2 code');
  }

  const data = b64Decode(ae2code.slice(4));
  if (data.length < 4) throw new Error('AE2 code too short');

  let pos = 0;
  const version = data[pos++];
  if (version !== 2 && version !== 3) throw new Error(`Unsupported AE2 version: ${version}`);

  const flags = data[pos++];
  const gameModeIdx = flags & 0x07;
  const hasGear = !!(flags & 0x08);
  const hasRune = !!(flags & 0x10);
  const hasRelic = !!(flags & 0x20);

  const linkLen = data[pos++];
  if (pos + linkLen > data.length) throw new Error('AE2 build link truncated');

  const linkBytes = data.slice(pos, pos + linkLen);
  pos += linkLen;

  const chatLink = '[&' + b64Encode(linkBytes) + ']';
  const gameMode = GAME_MODES[gameModeIdx] || 'PvE';

  const result = { version, chatLink, gameMode, gear: {}, runeId: 0, relicId: 0 };
  let gearMask = 0;

  if (hasGear) {
    if (pos + 2 > data.length) throw new Error('AE2 gear data truncated');
    gearMask = readU16(data, pos); pos += 2;

    const slotCount = popcount16(gearMask);
    if (pos + slotCount * 2 > data.length) throw new Error('AE2 gear stats truncated');

    for (let i = 0; i < 16; i++) {
      if (!(gearMask & (1 << i))) continue;
      const statId = readU16(data, pos); pos += 2;
      result.gear[SLOT_NAMES[i]] = { statId };
    }
  }

  if (hasRune) {
    if (pos + 4 > data.length) throw new Error('AE2 rune data truncated');
    result.runeId = readU32(data, pos); pos += 4;
  }

  if (hasRelic) {
    if (pos + 4 > data.length) throw new Error('AE2 relic data truncated');
    result.relicId = readU32(data, pos); pos += 4;
  }

  if (hasGear) {
    for (let i = WEAPON_SLOT_START; i <= WEAPON_SLOT_END; i++) {
      if (!(gearMask & (1 << i))) continue;

      const needed = version >= 3 ? 10 : 6;
      if (pos + needed > data.length) break;

      const sigilId = readU32(data, pos); pos += 4;
      const weaponTypeId = readU16(data, pos); pos += 2;
      let sigil2Id = 0;
      if (version >= 3) { sigil2Id = readU32(data, pos); pos += 4; }

      const gs = result.gear[SLOT_NAMES[i]];
      if (gs) {
        gs.sigilId = sigilId;
        gs.weaponType = WEAPON_ID_TO_TYPE[weaponTypeId] || '';
        gs.sigil2Id = sigil2Id;
      }
    }
  }

  return result;
}
