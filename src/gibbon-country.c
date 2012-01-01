/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-country
 * @short_description: Class representing a country.
 *
 * Since: 0.1.0
 *
 * This class represents a country.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon-country.h"
#include "gibbon-app.h"

typedef struct _GibbonCountryPrivate GibbonCountryPrivate;
struct _GibbonCountryPrivate {
        const gchar *alpha2;
        const gchar *name;
        const GdkPixbuf *pixbuf;
};

#define GIBBON_COUNTRY_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_COUNTRY, GibbonCountryPrivate))

G_DEFINE_TYPE (GibbonCountry, gibbon_country, G_TYPE_OBJECT)

#define GIBBON_COUNTRY_MAX 26 * 26
#define GIBBON_COUNTRY_FALLBACK (26 * ('x' - 'a')) + 'y' - 'a'

static const gchar * const country_codes[GIBBON_COUNTRY_MAX] = {
        "aa",
        "ab",
        "ac",
        "ad",
        "ae",
        "af",
        "ag",
        "ah",
        "ai",
        "aj",
        "ak",
        "al",
        "am",
        "an",
        "ao",
        "ap",
        "aq",
        "ar",
        "as",
        "at",
        "au",
        "av",
        "aw",
        "ax",
        "ay",
        "az",
        "ba",
        "bb",
        "bc",
        "bd",
        "be",
        "bf",
        "bg",
        "bh",
        "bi",
        "bj",
        "bk",
        "bl",
        "bm",
        "bn",
        "bo",
        "bp",
        "bq",
        "br",
        "bs",
        "bt",
        "bu",
        "bv",
        "bw",
        "bx",
        "by",
        "bz",
        "ca",
        "cb",
        "cc",
        "cd",
        "ce",
        "cf",
        "cg",
        "ch",
        "ci",
        "cj",
        "ck",
        "cl",
        "cm",
        "cn",
        "co",
        "cp",
        "cq",
        "cr",
        "cs",
        "ct",
        "cu",
        "cv",
        "cw",
        "cx",
        "cy",
        "cz",
        "da",
        "db",
        "dc",
        "dd",
        "de",
        "df",
        "dg",
        "dh",
        "di",
        "dj",
        "dk",
        "dl",
        "dm",
        "dn",
        "do",
        "dp",
        "dq",
        "dr",
        "ds",
        "dt",
        "du",
        "dv",
        "dw",
        "dx",
        "dy",
        "dz",
        "ea",
        "eb",
        "ec",
        "ed",
        "ee",
        "ef",
        "eg",
        "eh",
        "ei",
        "ej",
        "ek",
        "el",
        "em",
        "en",
        "eo",
        "ep",
        "eq",
        "er",
        "es",
        "et",
        "eu",
        "ev",
        "ew",
        "ex",
        "ey",
        "ez",
        "fa",
        "fb",
        "fc",
        "fd",
        "fe",
        "ff",
        "fg",
        "fh",
        "fi",
        "fj",
        "fk",
        "fl",
        "fm",
        "fn",
        "fo",
        "fp",
        "fq",
        "fr",
        "fs",
        "ft",
        "fu",
        "fv",
        "fw",
        "fx",
        "fy",
        "fz",
        "ga",
        "gb",
        "gc",
        "gd",
        "ge",
        "gf",
        "gg",
        "gh",
        "gi",
        "gj",
        "gk",
        "gl",
        "gm",
        "gn",
        "go",
        "gp",
        "gq",
        "gr",
        "gs",
        "gt",
        "gu",
        "gv",
        "gw",
        "gx",
        "gy",
        "gz",
        "ha",
        "hb",
        "hc",
        "hd",
        "he",
        "hf",
        "hg",
        "hh",
        "hi",
        "hj",
        "hk",
        "hl",
        "hm",
        "hn",
        "ho",
        "hp",
        "hq",
        "hr",
        "hs",
        "ht",
        "hu",
        "hv",
        "hw",
        "hx",
        "hy",
        "hz",
        "ia",
        "ib",
        "ic",
        "id",
        "ie",
        "if",
        "ig",
        "ih",
        "ii",
        "ij",
        "ik",
        "il",
        "im",
        "in",
        "io",
        "ip",
        "iq",
        "ir",
        "is",
        "it",
        "iu",
        "iv",
        "iw",
        "ix",
        "iy",
        "iz",
        "ja",
        "jb",
        "jc",
        "jd",
        "je",
        "jf",
        "jg",
        "jh",
        "ji",
        "jj",
        "jk",
        "jl",
        "jm",
        "jn",
        "jo",
        "jp",
        "jq",
        "jr",
        "js",
        "jt",
        "ju",
        "jv",
        "jw",
        "jx",
        "jy",
        "jz",
        "ka",
        "kb",
        "kc",
        "kd",
        "ke",
        "kf",
        "kg",
        "kh",
        "ki",
        "kj",
        "kk",
        "kl",
        "km",
        "kn",
        "ko",
        "kp",
        "kq",
        "kr",
        "ks",
        "kt",
        "ku",
        "kv",
        "kw",
        "kx",
        "ky",
        "kz",
        "la",
        "lb",
        "lc",
        "ld",
        "le",
        "lf",
        "lg",
        "lh",
        "li",
        "lj",
        "lk",
        "ll",
        "lm",
        "ln",
        "lo",
        "lp",
        "lq",
        "lr",
        "ls",
        "lt",
        "lu",
        "lv",
        "lw",
        "lx",
        "ly",
        "lz",
        "ma",
        "mb",
        "mc",
        "md",
        "me",
        "mf",
        "mg",
        "mh",
        "mi",
        "mj",
        "mk",
        "ml",
        "mm",
        "mn",
        "mo",
        "mp",
        "mq",
        "mr",
        "ms",
        "mt",
        "mu",
        "mv",
        "mw",
        "mx",
        "my",
        "mz",
        "na",
        "nb",
        "nc",
        "nd",
        "ne",
        "nf",
        "ng",
        "nh",
        "ni",
        "nj",
        "nk",
        "nl",
        "nm",
        "nn",
        "no",
        "np",
        "nq",
        "nr",
        "ns",
        "nt",
        "nu",
        "nv",
        "nw",
        "nx",
        "ny",
        "nz",
        "oa",
        "ob",
        "oc",
        "od",
        "oe",
        "of",
        "og",
        "oh",
        "oi",
        "oj",
        "ok",
        "ol",
        "om",
        "on",
        "oo",
        "op",
        "oq",
        "or",
        "os",
        "ot",
        "ou",
        "ov",
        "ow",
        "ox",
        "oy",
        "oz",
        "pa",
        "pb",
        "pc",
        "pd",
        "pe",
        "pf",
        "pg",
        "ph",
        "pi",
        "pj",
        "pk",
        "pl",
        "pm",
        "pn",
        "po",
        "pp",
        "pq",
        "pr",
        "ps",
        "pt",
        "pu",
        "pv",
        "pw",
        "px",
        "py",
        "pz",
        "qa",
        "qb",
        "qc",
        "qd",
        "qe",
        "qf",
        "qg",
        "qh",
        "qi",
        "qj",
        "qk",
        "ql",
        "qm",
        "qn",
        "qo",
        "qp",
        "qq",
        "qr",
        "qs",
        "qt",
        "qu",
        "qv",
        "qw",
        "qx",
        "qy",
        "qz",
        "ra",
        "rb",
        "rc",
        "rd",
        "re",
        "rf",
        "rg",
        "rh",
        "ri",
        "rj",
        "rk",
        "rl",
        "rm",
        "rn",
        "ro",
        "rp",
        "rq",
        "rr",
        "rs",
        "rt",
        "ru",
        "rv",
        "rw",
        "rx",
        "ry",
        "rz",
        "sa",
        "sb",
        "sc",
        "sd",
        "se",
        "sf",
        "sg",
        "sh",
        "si",
        "sj",
        "sk",
        "sl",
        "sm",
        "sn",
        "so",
        "sp",
        "sq",
        "sr",
        "ss",
        "st",
        "su",
        "sv",
        "sw",
        "sx",
        "sy",
        "sz",
        "ta",
        "tb",
        "tc",
        "td",
        "te",
        "tf",
        "tg",
        "th",
        "ti",
        "tj",
        "tk",
        "tl",
        "tm",
        "tn",
        "to",
        "tp",
        "tq",
        "tr",
        "ts",
        "tt",
        "tu",
        "tv",
        "tw",
        "tx",
        "ty",
        "tz",
        "ua",
        "ub",
        "uc",
        "ud",
        "ue",
        "uf",
        "ug",
        "uh",
        "ui",
        "uj",
        "uk",
        "ul",
        "um",
        "un",
        "uo",
        "up",
        "uq",
        "ur",
        "us",
        "ut",
        "uu",
        "uv",
        "uw",
        "ux",
        "uy",
        "uz",
        "va",
        "vb",
        "vc",
        "vd",
        "ve",
        "vf",
        "vg",
        "vh",
        "vi",
        "vj",
        "vk",
        "vl",
        "vm",
        "vn",
        "vo",
        "vp",
        "vq",
        "vr",
        "vs",
        "vt",
        "vu",
        "vv",
        "vw",
        "vx",
        "vy",
        "vz",
        "wa",
        "wb",
        "wc",
        "wd",
        "we",
        "wf",
        "wg",
        "wh",
        "wi",
        "wj",
        "wk",
        "wl",
        "wm",
        "wn",
        "wo",
        "wp",
        "wq",
        "wr",
        "ws",
        "wt",
        "wu",
        "wv",
        "ww",
        "wx",
        "wy",
        "wz",
        "xa",
        "xb",
        "xc",
        "xd",
        "xe",
        "xf",
        "xg",
        "xh",
        "xi",
        "xj",
        "xk",
        "xl",
        "xm",
        "xn",
        "xo",
        "xp",
        "xq",
        "xr",
        "xs",
        "xt",
        "xu",
        "xv",
        "xw",
        "xx",
        "xy",
        "xz",
        "ya",
        "yb",
        "yc",
        "yd",
        "ye",
        "yf",
        "yg",
        "yh",
        "yi",
        "yj",
        "yk",
        "yl",
        "ym",
        "yn",
        "yo",
        "yp",
        "yq",
        "yr",
        "ys",
        "yt",
        "yu",
        "yv",
        "yw",
        "yx",
        "yy",
        "yz",
        "za",
        "zb",
        "zc",
        "zd",
        "ze",
        "zf",
        "zg",
        "zh",
        "zi",
        "zj",
        "zk",
        "zl",
        "zm",
        "zn",
        "zo",
        "zp",
        "zq",
        "zr",
        "zs",
        "zt",
        "zu",
        "zv",
        "zw",
        "zx",
        "zy",
        "zz",
};

static const gchar * const country_names[GIBBON_COUNTRY_MAX] = {
        /* aa */
        NULL,
        /* ab */
        NULL,
        /* ac */
        NULL,
        /* ad */
        N_("Andorra"),
        /* ae */
        N_("United Arab Emirates"),
        /* af */
        N_("Afghanistan"),
        /* ag */
        N_("Antigua and Barbuda"),
        /* ah */
        NULL,
        /* ai */
        N_("Anguilla"),
        /* aj */
        NULL,
        /* ak */
        NULL,
        /* al */
        N_("Albania"),
        /* am */
        N_("Armenia"),
        /* an */
        N_("Netherlands Antilles"),
        /* ao */
        N_("Angola"),
        /* ap */
        N_("Asian Pacific Region"),
        /* aq */
        N_("Antarctica"),
        /* ar */
        N_("Argentina"),
        /* as */
        N_("American Samoa"),
        /* at */
        N_("Austria"),
        /* au */
        N_("Australia"),
        /* av */
        NULL,
        /* aw */
        N_("Aruba"),
        /* ax */
        N_("Aland Islands"),
        /* ay */
        NULL,
        /* az */
        N_("Azerbaijan"),
        /* ba */
        N_("Bosnia and Herzegovina"),
        /* bb */
        N_("Barbados"),
        /* bc */
        NULL,
        /* bd */
        N_("Bangladesh"),
        /* be */
        N_("Belgium"),
        /* bf */
        N_("Burkina Faso"),
        /* bg */
        N_("Bulgaria"),
        /* bh */
        N_("Bahrain"),
        /* bi */
        N_("Burundi"),
        /* bj */
        N_("Benin"),
        /* bk */
        NULL,
        /* bl */
        N_("Saint Barthelemy"),
        /* bm */
        N_("Bermuda"),
        /* bn */
        N_("Brunei Darussalam"),
        /* bo */
        N_("Bolivia"),
        /* bp */
        NULL,
        /* bq */
        N_("Bonaire, Saint Eustatius and Saba"),
        /* br */
        N_("Brazil"),
        /* bs */
        N_("Bahamas"),
        /* bt */
        N_("Bhutan"),
        /* bu */
        NULL,
        /* bv */
        N_("Bouvet Island"),
        /* bw */
        N_("Botswana"),
        /* bx */
        NULL,
        /* by */
        N_("Belarus"),
        /* bz */
        N_("Belize"),
        /* ca */
        N_("Canada"),
        /* cb */
        NULL,
        /* cc */
        N_("Cocos (Keeling) Islands"),
        /* cd */
        N_("Democratic Republic of the Congo"),
        /* ce */
        NULL,
        /* cf */
        N_("Central African Republic"),
        /* cg */
        N_("Congo"),
        /* ch */
        N_("Switzerland"),
        /* ci */
        N_("Ivory Coast"),
        /* cj */
        NULL,
        /* ck */
        N_("Cook Islands"),
        /* cl */
        N_("Chile"),
        /* cm */
        N_("Cameroon"),
        /* cn */
        N_("China"),
        /* co */
        N_("Colombia"),
        /* cp */
        NULL,
        /* cq */
        NULL,
        /* cr */
        N_("Costa Rica"),
        /* cs */
        NULL,
        /* ct */
        NULL,
        /* cu */
        N_("Cuba"),
        /* cv */
        N_("Cape Verde"),
        /* cw */
        N_("Curacao"),
        /* cx */
        N_("Christmas Island"),
        /* cy */
        N_("Cyprus"),
        /* cz */
        N_("Czech Republic"),
        /* da */
        NULL,
        /* db */
        NULL,
        /* dc */
        NULL,
        /* dd */
        NULL,
        /* de */
        N_("Germany"),
        /* df */
        NULL,
        /* dg */
        NULL,
        /* dh */
        NULL,
        /* di */
        NULL,
        /* dj */
        N_("Djibouti"),
        /* dk */
        N_("Denmark"),
        /* dl */
        NULL,
        /* dm */
        N_("Dominica"),
        /* dn */
        NULL,
        /* do */
        N_("Dominican Republic"),
        /* dp */
        NULL,
        /* dq */
        NULL,
        /* dr */
        NULL,
        /* ds */
        NULL,
        /* dt */
        NULL,
        /* du */
        NULL,
        /* dv */
        NULL,
        /* dw */
        NULL,
        /* dx */
        NULL,
        /* dy */
        NULL,
        /* dz */
        N_("Algeria"),
        /* ea */
        NULL,
        /* eb */
        NULL,
        /* ec */
        N_("Ecuador"),
        /* ed */
        NULL,
        /* ee */
        N_("Estonia"),
        /* ef */
        NULL,
        /* eg */
        N_("Egypt"),
        /* eh */
        N_("Western Sahara"),
        /* ei */
        NULL,
        /* ej */
        NULL,
        /* ek */
        NULL,
        /* el */
        NULL,
        /* em */
        NULL,
        /* en */
        NULL,
        /* eo */
        NULL,
        /* ep */
        NULL,
        /* eq */
        NULL,
        /* er */
        N_("Eritrea"),
        /* es */
        N_("Spain"),
        /* et */
        N_("Ethiopia"),
        /* eu */
        N_("European Union"),
        /* ev */
        NULL,
        /* ew */
        NULL,
        /* ex */
        NULL,
        /* ey */
        NULL,
        /* ez */
        NULL,
        /* fa */
        NULL,
        /* fb */
        NULL,
        /* fc */
        NULL,
        /* fd */
        NULL,
        /* fe */
        NULL,
        /* ff */
        NULL,
        /* fg */
        NULL,
        /* fh */
        NULL,
        /* fi */
        N_("Finland"),
        /* fj */
        N_("Fiji"),
        /* fk */
        N_("Falkland Islands (Malvinas)"),
        /* fl */
        NULL,
        /* fm */
        N_("Micronesia"),
        /* fn */
        NULL,
        /* fo */
        N_("Faroe Islands"),
        /* fp */
        NULL,
        /* fq */
        NULL,
        /* fr */
        N_("France"),
        /* fs */
        NULL,
        /* ft */
        NULL,
        /* fu */
        NULL,
        /* fv */
        NULL,
        /* fw */
        NULL,
        /* fx */
        N_("France, Metropolitan"),
        /* fy */
        NULL,
        /* fz */
        NULL,
        /* ga */
        N_("Gabon"),
        /* gb */
        N_("United Kingdom"),
        /* gc */
        NULL,
        /* gd */
        N_("Grenada"),
        /* ge */
        N_("Georgia"),
        /* gf */
        N_("French Guiana"),
        /* gg */
        N_("Guernsey"),
        /* gh */
        N_("Ghana"),
        /* gi */
        N_("Gibraltar"),
        /* gj */
        NULL,
        /* gk */
        NULL,
        /* gl */
        N_("Greenland"),
        /* gm */
        N_("Gambia"),
        /* gn */
        N_("Guinea"),
        /* go */
        NULL,
        /* gp */
        N_("Guadeloupe"),
        /* gq */
        N_("Equatorial Guinea"),
        /* gr */
        N_("Greece"),
        /* gs */
        N_("South Georgia and the South Sandwich Islands"),
        /* gt */
        N_("Guatemala"),
        /* gu */
        N_("Guam"),
        /* gv */
        NULL,
        /* gw */
        N_("Guinea-Bissau"),
        /* gx */
        NULL,
        /* gy */
        N_("Guyana"),
        /* gz */
        NULL,
        /* ha */
        NULL,
        /* hb */
        NULL,
        /* hc */
        NULL,
        /* hd */
        NULL,
        /* he */
        NULL,
        /* hf */
        NULL,
        /* hg */
        NULL,
        /* hh */
        NULL,
        /* hi */
        NULL,
        /* hj */
        NULL,
        /* hk */
        N_("Hong Kong"),
        /* hl */
        NULL,
        /* hm */
        N_("Heard Island and Mcdonald Islands"),
        /* hn */
        N_("Honduras"),
        /* ho */
        NULL,
        /* hp */
        NULL,
        /* hq */
        NULL,
        /* hr */
        N_("Croatia"),
        /* hs */
        NULL,
        /* ht */
        N_("Haiti"),
        /* hu */
        N_("Hungary"),
        /* hv */
        NULL,
        /* hw */
        NULL,
        /* hx */
        NULL,
        /* hy */
        NULL,
        /* hz */
        NULL,
        /* ia */
        NULL,
        /* ib */
        NULL,
        /* ic */
        NULL,
        /* id */
        N_("Indonesia"),
        /* ie */
        N_("Ireland"),
        /* if */
        NULL,
        /* ig */
        NULL,
        /* ih */
        NULL,
        /* ii */
        NULL,
        /* ij */
        NULL,
        /* ik */
        NULL,
        /* il */
        N_("Israel"),
        /* im */
        N_("Isle of Man"),
        /* in */
        N_("India"),
        /* io */
        N_("British Indian Ocean Territory"),
        /* ip */
        NULL,
        /* iq */
        N_("Iraq"),
        /* ir */
        N_("Iran"),
        /* is */
        N_("Iceland"),
        /* it */
        N_("Italy"),
        /* iu */
        NULL,
        /* iv */
        NULL,
        /* iw */
        NULL,
        /* ix */
        NULL,
        /* iy */
        NULL,
        /* iz */
        NULL,
        /* ja */
        NULL,
        /* jb */
        NULL,
        /* jc */
        NULL,
        /* jd */
        NULL,
        /* je */
        N_("Jersey"),
        /* jf */
        NULL,
        /* jg */
        NULL,
        /* jh */
        NULL,
        /* ji */
        NULL,
        /* jj */
        NULL,
        /* jk */
        NULL,
        /* jl */
        NULL,
        /* jm */
        N_("Jamaica"),
        /* jn */
        NULL,
        /* jo */
        N_("Jordan"),
        /* jp */
        N_("Japan"),
        /* jq */
        NULL,
        /* jr */
        NULL,
        /* js */
        NULL,
        /* jt */
        NULL,
        /* ju */
        NULL,
        /* jv */
        NULL,
        /* jw */
        NULL,
        /* jx */
        NULL,
        /* jy */
        NULL,
        /* jz */
        NULL,
        /* ka */
        NULL,
        /* kb */
        NULL,
        /* kc */
        NULL,
        /* kd */
        NULL,
        /* ke */
        N_("Kenya"),
        /* kf */
        NULL,
        /* kg */
        N_("Kyrgyzstan"),
        /* kh */
        N_("Cambodia"),
        /* ki */
        N_("Kiribati"),
        /* kj */
        NULL,
        /* kk */
        NULL,
        /* kl */
        NULL,
        /* km */
        N_("Comoros"),
        /* kn */
        N_("Saint Kitts and Nevis"),
        /* ko */
        NULL,
        /* kp */
        N_("North Korea"),
        /* kq */
        NULL,
        /* kr */
        N_("South Korea"),
        /* ks */
        NULL,
        /* kt */
        NULL,
        /* ku */
        NULL,
        /* kv */
        NULL,
        /* kw */
        N_("Kuwait"),
        /* kx */
        NULL,
        /* ky */
        N_("Cayman Islands"),
        /* kz */
        N_("Kazakhstan"),
        /* la */
        N_("Laos"),
        /* lb */
        N_("Lebanon"),
        /* lc */
        N_("Saint Lucia"),
        /* ld */
        NULL,
        /* le */
        NULL,
        /* lf */
        NULL,
        /* lg */
        NULL,
        /* lh */
        NULL,
        /* li */
        N_("Liechtenstein"),
        /* lj */
        NULL,
        /* lk */
        N_("Sri Lanka"),
        /* ll */
        NULL,
        /* lm */
        NULL,
        /* ln */
        NULL,
        /* lo */
        NULL,
        /* lp */
        NULL,
        /* lq */
        NULL,
        /* lr */
        N_("Liberia"),
        /* ls */
        N_("Lesotho"),
        /* lt */
        N_("Lithuania"),
        /* lu */
        N_("Luxembourg"),
        /* lv */
        N_("Latvia"),
        /* lw */
        NULL,
        /* lx */
        NULL,
        /* ly */
        N_("Libya"),
        /* lz */
        NULL,
        /* ma */
        N_("Morocco"),
        /* mb */
        NULL,
        /* mc */
        N_("Monaco"),
        /* md */
        N_("Moldova"),
        /* me */
        N_("Montenegro"),
        /* mf */
        N_("Saint Martin (French part)"),
        /* mg */
        N_("Madagascar"),
        /* mh */
        N_("Marshall Islands"),
        /* mi */
        NULL,
        /* mj */
        NULL,
        /* mk */
        N_("Macedonia"),
        /* ml */
        N_("Mali"),
        /* mm */
        N_("Myanmar"),
        /* mn */
        N_("Mongolia"),
        /* mo */
        N_("Macao"),
        /* mp */
        N_("Northern Mariana Islands"),
        /* mq */
        N_("Martinique"),
        /* mr */
        N_("Mauritania"),
        /* ms */
        N_("Montserrat"),
        /* mt */
        N_("Malta"),
        /* mu */
        N_("Mauritius"),
        /* mv */
        N_("Maldives"),
        /* mw */
        N_("Malawi"),
        /* mx */
        N_("Mexico"),
        /* my */
        N_("Malaysia"),
        /* mz */
        N_("Mozambique"),
        /* na */
        N_("Namibia"),
        /* nb */
        NULL,
        /* nc */
        N_("New Caledonia"),
        /* nd */
        NULL,
        /* ne */
        N_("Niger"),
        /* nf */
        N_("Norfolk Island"),
        /* ng */
        N_("Nigeria"),
        /* nh */
        NULL,
        /* ni */
        N_("Nicaragua"),
        /* nj */
        NULL,
        /* nk */
        NULL,
        /* nl */
        N_("Netherlands"),
        /* nm */
        NULL,
        /* nn */
        NULL,
        /* no */
        N_("Norway"),
        /* np */
        N_("Nepal"),
        /* nq */
        NULL,
        /* nr */
        N_("Nauru"),
        /* ns */
        NULL,
        /* nt */
        NULL,
        /* nu */
        N_("Niue"),
        /* nv */
        NULL,
        /* nw */
        NULL,
        /* nx */
        NULL,
        /* ny */
        NULL,
        /* nz */
        N_("New Zealand"),
        /* oa */
        NULL,
        /* ob */
        NULL,
        /* oc */
        NULL,
        /* od */
        NULL,
        /* oe */
        NULL,
        /* of */
        NULL,
        /* og */
        NULL,
        /* oh */
        NULL,
        /* oi */
        NULL,
        /* oj */
        NULL,
        /* ok */
        NULL,
        /* ol */
        NULL,
        /* om */
        N_("Oman"),
        /* on */
        NULL,
        /* oo */
        NULL,
        /* op */
        NULL,
        /* oq */
        NULL,
        /* or */
        NULL,
        /* os */
        NULL,
        /* ot */
        NULL,
        /* ou */
        NULL,
        /* ov */
        NULL,
        /* ow */
        NULL,
        /* ox */
        NULL,
        /* oy */
        NULL,
        /* oz */
        NULL,
        /* pa */
        N_("Panama"),
        /* pb */
        NULL,
        /* pc */
        NULL,
        /* pd */
        NULL,
        /* pe */
        N_("Peru"),
        /* pf */
        N_("French Polynesia"),
        /* pg */
        N_("Papua New Guinea"),
        /* ph */
        N_("Philippines"),
        /* pi */
        NULL,
        /* pj */
        NULL,
        /* pk */
        N_("Pakistan"),
        /* pl */
        N_("Poland"),
        /* pm */
        N_("Saint Pierre and Miquelon"),
        /* pn */
        N_("Pitcairn"),
        /* po */
        NULL,
        /* pp */
        NULL,
        /* pq */
        NULL,
        /* pr */
        N_("Puerto Rico"),
        /* ps */
        N_("Palestinian Territory"),
        /* pt */
        N_("Portugal"),
        /* pu */
        NULL,
        /* pv */
        NULL,
        /* pw */
        N_("Palau"),
        /* px */
        NULL,
        /* py */
        N_("Paraguay"),
        /* pz */
        NULL,
        /* qa */
        N_("Qatar"),
        /* qb */
        NULL,
        /* qc */
        NULL,
        /* qd */
        NULL,
        /* qe */
        NULL,
        /* qf */
        NULL,
        /* qg */
        NULL,
        /* qh */
        NULL,
        /* qi */
        NULL,
        /* qj */
        NULL,
        /* qk */
        NULL,
        /* ql */
        NULL,
        /* qm */
        NULL,
        /* qn */
        NULL,
        /* qo */
        NULL,
        /* qp */
        NULL,
        /* qq */
        NULL,
        /* qr */
        NULL,
        /* qs */
        NULL,
        /* qt */
        NULL,
        /* qu */
        NULL,
        /* qv */
        NULL,
        /* qw */
        NULL,
        /* qx */
        NULL,
        /* qy */
        NULL,
        /* qz */
        NULL,
        /* ra */
        NULL,
        /* rb */
        NULL,
        /* rc */
        NULL,
        /* rd */
        NULL,
        /* re */
        N_("Reunion"),
        /* rf */
        NULL,
        /* rg */
        NULL,
        /* rh */
        NULL,
        /* ri */
        NULL,
        /* rj */
        NULL,
        /* rk */
        NULL,
        /* rl */
        NULL,
        /* rm */
        NULL,
        /* rn */
        NULL,
        /* ro */
        N_("Romania"),
        /* rp */
        NULL,
        /* rq */
        NULL,
        /* rr */
        NULL,
        /* rs */
        N_("Serbia"),
        /* rt */
        NULL,
        /* ru */
        N_("Russia"),
        /* rv */
        NULL,
        /* rw */
        N_("Rwanda"),
        /* rx */
        NULL,
        /* ry */
        NULL,
        /* rz */
        NULL,
        /* sa */
        N_("Saudi Arabia"),
        /* sb */
        N_("Solomon Islands"),
        /* sc */
        N_("Seychelles"),
        /* sd */
        N_("Sudan"),
        /* se */
        N_("Sweden"),
        /* sf */
        NULL,
        /* sg */
        N_("Singapore"),
        /* sh */
        N_("Saint Helena, Ascension and Tristan da Cunha"),
        /* si */
        N_("Slovenia"),
        /* sj */
        N_("Svalbard and Jan Mayen"),
        /* sk */
        N_("Slovakia"),
        /* sl */
        N_("Sierra Leone"),
        /* sm */
        N_("San Marino"),
        /* sn */
        N_("Senegal"),
        /* so */
        N_("Somalia"),
        /* sp */
        NULL,
        /* sq */
        NULL,
        /* sr */
        N_("Suriname"),
        /* ss */
        N_("South Sudan"),
        /* st */
        N_("Sao Tome and Principe"),
        /* su */
        NULL,
        /* sv */
        N_("El Salvador"),
        /* sw */
        NULL,
        /* sx */
        N_("Sint Maarten (Dutch part)"),
        /* sy */
        N_("Syrian Arab Republic"),
        /* sz */
        N_("Swaziland"),
        /* ta */
        NULL,
        /* tb */
        NULL,
        /* tc */
        N_("Turks and Caicos Islands"),
        /* td */
        N_("Chad"),
        /* te */
        NULL,
        /* tf */
        N_("French Southern Territories"),
        /* tg */
        N_("Togo"),
        /* th */
        N_("Thailand"),
        /* ti */
        NULL,
        /* tj */
        N_("Tajikistan"),
        /* tk */
        N_("Tokelau"),
        /* tl */
        N_("Timor-Leste"),
        /* tm */
        N_("Turkmenistan"),
        /* tn */
        N_("Tunisia"),
        /* to */
        N_("Tonga"),
        /* tp */
        NULL,
        /* tq */
        NULL,
        /* tr */
        N_("Turkey"),
        /* ts */
        NULL,
        /* tt */
        N_("Trinidad and Tobago"),
        /* tu */
        NULL,
        /* tv */
        N_("Tuvalu"),
        /* tw */
        N_("Taiwan, Province of China"),
        /* tx */
        NULL,
        /* ty */
        NULL,
        /* tz */
        N_("Tanzania, United Republic of"),
        /* ua */
        N_("Ukraine"),
        /* ub */
        NULL,
        /* uc */
        NULL,
        /* ud */
        NULL,
        /* ue */
        NULL,
        /* uf */
        NULL,
        /* ug */
        N_("Uganda"),
        /* uh */
        NULL,
        /* ui */
        NULL,
        /* uj */
        NULL,
        /* uk */
        NULL,
        /* ul */
        NULL,
        /* um */
        N_("United States Minor Outlying Islands"),
        /* un */
        NULL,
        /* uo */
        NULL,
        /* up */
        NULL,
        /* uq */
        NULL,
        /* ur */
        NULL,
        /* us */
        N_("United States"),
        /* ut */
        NULL,
        /* uu */
        NULL,
        /* uv */
        NULL,
        /* uw */
        NULL,
        /* ux */
        NULL,
        /* uy */
        N_("Uruguay"),
        /* uz */
        N_("Uzbekistan"),
        /* va */
        N_("Vatican"),
        /* vb */
        NULL,
        /* vc */
        N_("Saint Vincent and the Grenadines"),
        /* vd */
        NULL,
        /* ve */
        N_("Venezuela"),
        /* vf */
        NULL,
        /* vg */
        N_("British Virgin Islands"),
        /* vh */
        NULL,
        /* vi */
        N_("U.S. Virgin Islands"),
        /* vj */
        NULL,
        /* vk */
        NULL,
        /* vl */
        NULL,
        /* vm */
        NULL,
        /* vn */
        N_("Vietnam"),
        /* vo */
        NULL,
        /* vp */
        NULL,
        /* vq */
        NULL,
        /* vr */
        NULL,
        /* vs */
        NULL,
        /* vt */
        NULL,
        /* vu */
        N_("Vanuatu"),
        /* vv */
        NULL,
        /* vw */
        NULL,
        /* vx */
        NULL,
        /* vy */
        NULL,
        /* vz */
        NULL,
        /* wa */
        NULL,
        /* wb */
        NULL,
        /* wc */
        NULL,
        /* wd */
        NULL,
        /* we */
        NULL,
        /* wf */
        N_("Wallis and Futuna"),
        /* wg */
        NULL,
        /* wh */
        NULL,
        /* wi */
        NULL,
        /* wj */
        NULL,
        /* wk */
        NULL,
        /* wl */
        NULL,
        /* wm */
        NULL,
        /* wn */
        NULL,
        /* wo */
        NULL,
        /* wp */
        NULL,
        /* wq */
        NULL,
        /* wr */
        NULL,
        /* ws */
        N_("Samoa"),
        /* wt */
        NULL,
        /* wu */
        NULL,
        /* wv */
        NULL,
        /* ww */
        NULL,
        /* wx */
        NULL,
        /* wy */
        NULL,
        /* wz */
        NULL,
        /* xa */
        NULL,
        /* xb */
        NULL,
        /* xc */
        NULL,
        /* xd */
        NULL,
        /* xe */
        NULL,
        /* xf */
        NULL,
        /* xg */
        NULL,
        /* xh */
        NULL,
        /* xi */
        NULL,
        /* xj */
        NULL,
        /* xk */
        NULL,
        /* xl */
        N_("On the server network"),
        /* xm */
        NULL,
        /* xn */
        NULL,
        /* xo */
        NULL,
        /* xp */
        NULL,
        /* xq */
        NULL,
        /* xr */
        NULL,
        /* xs */
        NULL,
        /* xt */
        NULL,
        /* xu */
        NULL,
        /* xv */
        NULL,
        /* xw */
        NULL,
        /* xx */
        NULL,
        /* xy */
        N_("Unknown country"),
        /* xz */
        NULL,
        /* ya */
        NULL,
        /* yb */
        NULL,
        /* yc */
        NULL,
        /* yd */
        NULL,
        /* ye */
        N_("Yemen"),
        /* yf */
        NULL,
        /* yg */
        NULL,
        /* yh */
        NULL,
        /* yi */
        NULL,
        /* yj */
        NULL,
        /* yk */
        NULL,
        /* yl */
        NULL,
        /* ym */
        NULL,
        /* yn */
        NULL,
        /* yo */
        NULL,
        /* yp */
        NULL,
        /* yq */
        NULL,
        /* yr */
        NULL,
        /* ys */
        NULL,
        /* yt */
        N_("Mayotte"),
        /* yu */
        NULL,
        /* yv */
        NULL,
        /* yw */
        NULL,
        /* yx */
        NULL,
        /* yy */
        NULL,
        /* yz */
        NULL,
        /* za */
        N_("South Africa"),
        /* zb */
        NULL,
        /* zc */
        NULL,
        /* zd */
        NULL,
        /* ze */
        NULL,
        /* zf */
        NULL,
        /* zg */
        NULL,
        /* zh */
        NULL,
        /* zi */
        NULL,
        /* zj */
        NULL,
        /* zk */
        NULL,
        /* zl */
        NULL,
        /* zm */
        N_("Zambia"),
        /* zn */
        NULL,
        /* zo */
        NULL,
        /* zp */
        NULL,
        /* zq */
        NULL,
        /* zr */
        NULL,
        /* zs */
        NULL,
        /* zt */
        NULL,
        /* zu */
        NULL,
        /* zv */
        NULL,
        /* zw */
        N_("Zimbabwe"),
        /* zx */
        NULL,
        /* zy */
        NULL,
        /* zz */
        N_("Invalid IP address")
};

static const GdkPixbuf *gibbon_country_pixbufs[GIBBON_COUNTRY_MAX];
static gboolean gibbon_country_pixbuf_initialized[GIBBON_COUNTRY_MAX];

static void 
gibbon_country_init (GibbonCountry *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_COUNTRY, GibbonCountryPrivate);

        self->priv->alpha2 = NULL;
        self->priv->name = NULL;
        self->priv->pixbuf = NULL;
}

static void
gibbon_country_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_country_parent_class)->finalize(object);
}

static void
gibbon_country_class_init (GibbonCountryClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonCountryPrivate));

        memset (gibbon_country_pixbufs, 0, sizeof gibbon_country_pixbufs);
        memset (gibbon_country_pixbuf_initialized, 0,
                sizeof gibbon_country_pixbuf_initialized);

        object_class->finalize = gibbon_country_finalize;
}

/**
 * gibbon_country_new:
 * @alpha2: The 2-letter ISO-3166 1 country code.
 *
 * Creates a new #GibbonCountry.
 *
 * Returns: The newly created #GibbonCountry or %NULL in case of failure.
 */
GibbonCountry *
gibbon_country_new (const gchar *alpha2)
{
        GibbonCountry *self = g_object_new (GIBBON_TYPE_COUNTRY, NULL);
        gint idx;
        gchar *path;
        gchar filename[7];

        if (!alpha2
            || alpha2[0] < 'a' || alpha2[0] > 'z'
            || alpha2[1] < 'a' || alpha2[1] > 'z') {
                idx = GIBBON_COUNTRY_FALLBACK;
        } else {
                idx = (alpha2[0] - 'a') * 26 + alpha2[1] - 'a';
        }

        if (!country_codes[idx])
                idx = GIBBON_COUNTRY_FALLBACK;

        self->priv->alpha2 = country_codes[idx];

        if (country_names[idx])
                self->priv->name = _(country_names[idx]);
        else
                self->priv->name = NULL;

        if (!gibbon_country_pixbuf_initialized[idx]) {
                gibbon_country_pixbuf_initialized[idx] = 1;
                snprintf (filename, 7, "%s.png", self->priv->alpha2);
                path = g_build_filename (gibbon_app_pixmaps_directory, "flags",
                                         "16x16", filename, NULL);
                gibbon_country_pixbufs[idx] =
                                gdk_pixbuf_new_from_file_at_size (path, -1, 16,
                                                                  NULL);
                g_free (path);
        }

        self->priv->pixbuf = gibbon_country_pixbufs[idx];

        return self;
}

const GdkPixbuf *
gibbon_country_get_pixbuf (const GibbonCountry *self)
{
        g_return_val_if_fail (GIBBON_IS_COUNTRY (self), NULL);

        return self->priv->pixbuf;
}

const gchar *
gibbon_country_get_name (const GibbonCountry *self)
{
        g_return_val_if_fail (GIBBON_IS_COUNTRY (self), NULL);

        return self->priv->name;
}

const gchar *
gibbon_country_get_alpha2 (const GibbonCountry *self)
{
        g_return_val_if_fail (GIBBON_IS_COUNTRY (self), NULL);

        return self->priv->alpha2;
}
