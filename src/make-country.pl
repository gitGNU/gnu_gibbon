#! /usr/bin/env perl
# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
#
# Gibbon is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gibbon is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.

use strict;

my $infile = $ARGV[0];
die "Usage: $0 ISOLIST [H|C]" unless defined $infile && length $infile;
my $type = $ARGV[1];
die "Usage: $0 ISOLIST [H|C]" 
        unless defined $type && length $type && ('H' eq $type || 'C' eq $type);

my %code2country = {
        ad => 'Andorra',
        ae => 'United Arab Emirates',
        af => 'Afghanistan',
        ag => 'Antigua and Barbuda',
        ai => 'Anguilla',
        al => 'Albania',
        am => 'Armenia',
        an => 'Netherlands Antilles',
        ao => 'Angola',
        aq => 'Antarctica',
        ar => 'Argentina',
        as => 'American Samoa',
        at => 'Austria',
        au => 'Australia',
        aw => 'Aruba',
        az => 'Azerbaijan',
        ba => 'Bosnia and Herzegovina',
        bb => 'Barbados',
        bd => 'Bangladesh',
        be => 'Belgium',
        bf => 'Burkina Faso',
        bg => 'Bulgaria',
        bh => 'Bahrain',
        bi => 'Burundi',
        bj => 'Benin',
        bm => 'Bermuda',
        bn => 'Brunei Darussalam',
        bo => 'Bolivia',
        br => 'Brazil',
        bs => 'Bahamas',
        bt => 'Bhutan',
        bv => 'Bouvet Island',
        bw => 'Botswana',
        by => 'Belarus',
        bz => 'Belize',
        ca => 'Canada',
        cc => 'Cocos (Keeling) Islands',
        cf => 'Central African Republic',
        cg => 'Congo',
        ch => 'Switzerland',
        ci => 'Cote D\'Ivoire',
        ck => 'Cook Islands',
        cl => 'Chile',
        cm => 'Cameroon',
        cn => 'China',
        co => 'Colombia',
        cr => 'Costa Rica',
        cu => 'Cuba',
        cv => 'Cape Verde',
        cx => 'Christmas Island',
        cy => 'Cyprus',
        cz => 'Czech Republic',
        de => 'Germany',
        dj => 'Djibouti',
        dk => 'Denmark',
        dm => 'Dominica',
        do => 'Dominican Republic',
        dz => 'Algeria',
        ec => 'Ecuador',
        ee => 'Estonia',
        eg => 'Egypt',
        eh => 'Western Sahara',
        er => 'Eritrea',
        es => 'Spain',
        et => 'Ethiopia',
        fi => 'Finland',
        fj => 'Fiji',
        fk => 'Falkland Islands (Malvinas)',
        fm => 'Micronesia',
        fo => 'Faroe Islands',
        fr => 'France',
        fx => 'France, Metropolitan',
        ga => 'Gabon',
        gb => 'United Kingdom',
        gd => 'Grenada',
        ge => 'Georgia',
        gf => 'French Guiana',
        gh => 'Ghana',
        gi => 'Gibraltar',
        gl => 'Greenland',
        gm => 'Gambia',
        gn => 'Guinea',
        gp => 'Guadeloupe',
        gq => 'Equatorial Guinea',
        gr => 'Greece',
        gs => 'South Georgia and the South Sandwich Islands',
        gt => 'Guatemala',
        gu => 'Guam',
        gw => 'Guinea-Bissau',
        gy => 'Guyana',
        hk => 'Hong Kong',
        hm => 'Heard Island and McDonald Islands',
        hn => 'Honduras',
        hr => 'Croatia',
        ht => 'Haiti',
        hu => 'Hungary',
        id => 'Indonesia',
        ie => 'Ireland',
        il => 'Israel',
        in => 'India',
        io => 'British Indian Ocean Territory',
        iq => 'Iraq',
        ir => 'Iran',
        is => 'Iceland',
        it => 'Italy',
        jm => 'Jamaica',
        jo => 'Jordan',
        jp => 'Japan',
        ke => 'Kenya',
        kg => 'Kyrgyzstan',
        kh => 'Cambodia',
        ki => 'Kiribati',
        km => 'Comoros',
        kn => 'Saint Kitts and Nevis',
        kp => 'North Korea',
        kr => 'South Korea',
        kw => 'Kuwait',
        ky => 'Cayman Islands',
        kz => 'Kazakhstan',
        la => 'Lao People\'s Democratic Republic',
        lb => 'Lebanon',
        lc => 'Saint Lucia',
        li => 'Liechtenstein',
        lk => 'Sri Lanka',
        lr => 'Liberia',
        ls => 'Lesotho',
        lt => 'Lithuania',
        lu => 'Luxembourg',
        lv => 'Latvia',
        ly => 'Libya',
        ma => 'Morocco',
        mc => 'Monaco',
        md => 'Moldova',
        mg => 'Madagascar',
        mh => 'Marshall Islands',
        mk => 'Macedonia',
        ml => 'Mali',
        mm => 'Myanmar',
        mn => 'Mongolia',
        mo => 'Macao',
        mp => 'Northern Mariana Islands',
        mq => 'Martinique',
        mr => 'Mauritania',
        ms => 'Montserrat',
        mt => 'Malta',
        mu => 'Mauritius',
        mv => 'Maldives',
        mw => 'Malawi',
        mx => 'Mexico',
        my => 'Malaysia',
        mz => 'Mozambique',
        na => 'Namibia',
        nc => 'New Caledonia',
        ne => 'Niger',
        nf => 'Norfolk Island',
        ng => 'Nigeria',
        ni => 'Nicaragua',
        nl => 'Netherlands',
        no => 'Norway',
        np => 'Nepal',
        nr => 'Nauru',
        nu => 'Niue',
        nz => 'New Zealand',
        om => 'Oman',
        pa => 'Panama',
        pe => 'Peru',
        pf => 'French Polynesia',
        pg => 'Papua New Guinea',
        ph => 'Philippines',
        pk => 'Pakistan',
        pl => 'Poland',
        pm => 'Saint Pierre and Miquelon',
        pn => 'Pitcairn',
        pr => 'Puerto Rico',
        pt => 'Portugal',
        pw => 'Palau',
        py => 'Paraguay',
        qa => 'Qatar',
        re => 'Reunion',
        ro => 'Romania',
        ru => 'Russia',
        rw => 'Rwanda',
        sa => 'Saudi Arabia',
        sb => 'Solomon Islands',
        sc => 'Seychelles',
        sd => 'Sudan',
        se => 'Sweden',
        sg => 'Singapore',
        sh => 'Saint Helena',
        si => 'Slovenia',
        sj => 'Svalbard and Jan Mayen',
        sk => 'Slovakia',
        sl => 'Sierra Leone',
        sm => 'San Marino',
        sn => 'Senegal',
        so => 'Somalia',
        sr => 'Suriname',
        st => 'Sao Tome and Principe',
        sv => 'El Salvador',
        sy => 'Syrian Arab Republic',
        sz => 'Swaziland',
        tc => 'Turks and Caicos Islands',
        td => 'Chad',
        tf => 'French Southern Territories',
        tg => 'Togo',
        th => 'Thailand',
        tj => 'Tajikistan',
        tk => 'Tokelau',
        tm => 'Turkmenistan',
        tn => 'Tunisia',
        to => 'Tonga',
        tp => '??',
        tr => 'Turkey',
        tt => 'Trinidad and Tobago',
        tv => 'Tuvalu',
        tw => 'Taiwan',
        tz => 'Tanzania',
        ua => 'Ukraine',
        ug => 'Uganda',
        uk => 'United Kingdom',
        um => 'United States Minor Outlying Islands',
        us => 'United States',
        uy => 'Uruguay',
        uz => 'Uzbekistan',
        va => 'Holy See (Vatican City State)',
        vc => 'Saint Vincent and the Grenadines',
        ve => 'Venezuela',
        vg => 'Virgin Islands, British',
        vi => 'Virgin Islands, U.S.',
        vn => 'Vietnam',
        vu => 'Vanuatu',
        wf => 'Wallis and Futuna',
        ws => 'Samoa',
        ye => 'Yemen',
        yt => 'Mayotte',
        yu => 'Former Yugoslavia',
        za => 'South Africa',
        zm => 'Zambia',
        zr => 'Zaire',
        zw => 'Zimbabwe'
};

my %pngs;
my $check_png;
if (exists $ENV{top_srcdir}) {
        my $flags_dir = "$ENV{top_srcdir}/pixmaps/flags/16x16";
        opendir DIR, $flags_dir or die "Cannot opendif `$flags_dir': $!\n";
        %pngs = map { s/\.png$//; $_ => 1 } grep { /\.png$/ } readdir DIR;
        closedir DIR;
        $check_png = 1;
}

open HANDLE, "<$infile"
        or die "Cannot open `$infile': $!\n";

my @lines = <HANDLE>;
my %codes;
my $lineno = 0;
foreach my $line (@lines) {
        ++$lineno;
        chomp $line;
        $_ = $line;
        my ($code, $ip) = split;
        die "$infile:$lineno: Unexpected country code `$code'\n"
                unless $code =~ /^[a-z][a-z]$/;

        if ($check_png) {
                unless (delete $pngs{$code}) {
                        my $flags_dir = "$ENV{top_srcdir}/pixmaps/flags/16x16";
                        warn "$infile:$lineno: Flag `$flags_dir/$code.png'"
                            . " is missing.\n";
                }
        }

        my @ip = split /\./, $ip, 4;
        die "$infile:$lineno: First octet must be 127, not $ip[0]!\n"
                unless 127 == $ip[0];
        die "$infile:$lineno: Second octet must be 0, not $ip[1]!\n"
                unless 0 == $ip[1];
        die "$infile:$lineno: Third octet must be in range 0-3!\n"
                unless $ip[2] >= 0 && $ip[2] <= 3;
        die "$infile:$lineno: Fourth octet must be in range 0-255!\n"
                unless $ip[3] >= 0 && $ip[3] <= 255;
        $codes{$ip[2] * 256 + $ip[3]} = $code;
}

print <<EOF;
/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file is generated, do not edit! Edit `isolist.txt' in the same
 * directory instead!
 */

EOF

if ('H' eq $type) {
        print <<EOF;
#ifndef _GIBBON_COUNTRIES_H
#define _GIBBON_COUNTRIES_H

enum GibbonCountry {
EOF

        my $max = 0;

        foreach my $num (sort { $a <=> $b } keys %codes) {
                print "        GIBBON_COUNTRY_$codes{$num} = $num,\n";
                $max = $num;
        }
        print <<EOF;
        GIBBON_COUNTRY_N_COUNTRIES
};

#endif
EOF
}
