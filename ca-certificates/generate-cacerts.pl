#!/usr/bin/perl -w

use diagnostics;
use Fcntl;

# Copyright (C) 2007, 2008 Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# generate-cacerts.pl generates a JKS keystore named 'cacerts' from
# OpenSSL's certificate bundle using OpenJDK's keytool.

# First extract each of OpenSSL's bundled certificates into its own
# aliased filename.
$file = $ARGV[1];
open(CERTS, $file);
@certs = <CERTS>;
close(CERTS);

$pem_file_count = 0;
$in_cert_block = 0;
$write_current_cert = 1;
foreach $cert (@certs)
{
    if ($cert =~ "Certificate:\n")
    {
        print "New certificate...\n";
    }        
    elsif ($cert =~ /Subject: /)
    {
        $_ = $cert;
        if ($cert =~ /personal-freemail/)
        {
            $cert_alias = "thawtepersonalfreemailca";
        }
        elsif ($cert =~ /personal-basic/)
        {
            $cert_alias = "thawtepersonalbasicca";
        }
        elsif ($cert =~ /personal-premium/)
        {
            $cert_alias = "thawtepersonalpremiumca";
        }
        elsif ($cert =~ /server-certs/)
        {
            $cert_alias = "thawteserverca";
        }
        elsif ($cert =~ /premium-server/)
        {
            $cert_alias = "thawtepremiumserverca";
        }
        elsif ($cert =~ /Class 1 Public Primary Certification Authority$/)
        {
            $cert_alias = "verisignclass1ca";
        }
        elsif ($cert =~ /Class 1 Public Primary Certification Authority - G2/)
        {
            $cert_alias = "verisignclass1g2ca";
        }
        elsif ($cert =~
               /VeriSign Class 1 Public Primary Certification Authority - G3/)
        {
            $cert_alias = "verisignclass1g3ca";
        }
        elsif ($cert =~ /Class 2 Public Primary Certification Authority$/)
        {
            $cert_alias = "verisignclass2ca";
        }
        elsif ($cert =~ /Class 2 Public Primary Certification Authority - G2/)
        {
            $cert_alias = "verisignclass2g2ca";
        }
        elsif ($cert =~
               /VeriSign Class 2 Public Primary Certification Authority - G3/)
        {
            $cert_alias = "verisignclass2g3ca";
        }
        elsif ($cert =~ /Class 3 Public Primary Certification Authority$/)
        {
            $cert_alias = "verisignclass3ca";
        }
        # Version 1 of Class 3 Public Primary Certification Authority
        # - G2 is added.  Version 3 is excluded.  See below.
        elsif ($cert =~ /Class 3 Public Primary Certification Authority - G2.*1998/)
        {
            $cert_alias = "verisignclass3g2ca";
        }
        elsif ($cert =~
               /VeriSign Class 3 Public Primary Certification Authority - G3/)
        {
            $cert_alias = "verisignclass3g3ca";
        }
        elsif ($cert =~
               /RSA Data Security.*Secure Server Certification Authority/)
        {
            $cert_alias = "rsaserverca";
        }
        elsif ($cert =~ /GTE CyberTrust Global Root/)
        {
            $cert_alias = "gtecybertrustglobalca";
        }
        elsif ($cert =~ /Baltimore CyberTrust Root/)
        {
            $cert_alias = "baltimorecybertrustca";
        }
        elsif ($cert =~ /www.entrust.net\/Client_CA_Info\/CPS/)
        {
            $cert_alias = "entrustclientca";
        }
        elsif ($cert =~ /www.entrust.net\/GCCA_CPS/)
        {
            $cert_alias = "entrustglobalclientca";
        }
        elsif ($cert =~ /www.entrust.net\/CPS_2048/)
        {
            $cert_alias = "entrust2048ca";
        }
        elsif ($cert =~ /www.entrust.net\/CPS incorp /)
        {
            $cert_alias = "entrustsslca";
        }
        elsif ($cert =~ /www.entrust.net\/SSL_CPS/)
        {
            $cert_alias = "entrustgsslca";
        }
        elsif ($cert =~ /The Go Daddy Group/)
        {
            $cert_alias = "godaddyclass2ca";
        }
        elsif ($cert =~ /Starfield Class 2 Certification Authority/)
        {
            $cert_alias = "starfieldclass2ca";
        }
        elsif ($cert =~ /ValiCert Class 2 Policy Validation Authority/)
        {
            $cert_alias = "valicertclass2ca";
        }
        elsif ($cert =~ /GeoTrust Global CA$/)
        {
            $cert_alias = "geotrustglobalca";
        }
        elsif ($cert =~ /Equifax Secure Certificate Authority/)
        {
            $cert_alias = "equifaxsecureca";
        }
        elsif ($cert =~ /Equifax Secure eBusiness CA-1/)
        {
            $cert_alias = "equifaxsecureebusinessca1";
        }
        elsif ($cert =~ /Equifax Secure eBusiness CA-2/)
        {
            $cert_alias = "equifaxsecureebusinessca2";
        }
        elsif ($cert =~ /Equifax Secure Global eBusiness CA-1/)
        {
            $cert_alias = "equifaxsecureglobalebusinessca1";
        }
        elsif ($cert =~ /Sonera Class1 CA/)
        {
            $cert_alias = "soneraclass1ca";
        }
        elsif ($cert =~ /Sonera Class2 CA/)
        {
            $cert_alias = "soneraclass2ca";
        }
        elsif ($cert =~ /AAA Certificate Services/)
        {
            $cert_alias = "comodoaaaca";
        }
        elsif ($cert =~ /AddTrust Class 1 CA Root/)
        {
            $cert_alias = "addtrustclass1ca";
        }
        elsif ($cert =~ /AddTrust External CA Root/)
        {
            $cert_alias = "addtrustexternalca";
        }
        elsif ($cert =~ /AddTrust Qualified CA Root/)
        {
            $cert_alias = "addtrustqualifiedca";
        }
        elsif ($cert =~ /UTN-USERFirst-Hardware/)
        {
            $cert_alias = "utnuserfirsthardwareca";
        }
        elsif ($cert =~ /UTN-USERFirst-Client Authentication and Email/)
        {
            $cert_alias = "utnuserfirstclientauthemailca";
        }
        elsif ($cert =~ /UTN - DATACorp SGC/)
        {
            $cert_alias = "utndatacorpsgcca";
        }
        elsif ($cert =~ /UTN-USERFirst-Object/)
        {
            $cert_alias = "utnuserfirstobjectca";
        }
        elsif ($cert =~ /America Online Root Certification Authority 1/)
        {
            $cert_alias = "aolrootca1";
        }
        elsif ($cert =~ /DigiCert Assured ID Root CA/)
        {
            $cert_alias = "digicertassuredidrootca";
        }
        elsif ($cert =~ /DigiCert Global Root CA/)
        {
            $cert_alias = "digicertglobalrootca";
        }
        elsif ($cert =~ /DigiCert High Assurance EV Root CA/)
        {
            $cert_alias = "digicerthighassuranceevrootca";
        }
        elsif ($cert =~ /GlobalSign Root CA$/)
        {
            $cert_alias = "globalsignca";
        }
        elsif ($cert =~ /GlobalSign Root CA - R2/)
        {
            $cert_alias = "globalsignr2ca";
        }
        elsif ($cert =~ /Elektronik.*Kas.*2005/)
        {
            $cert_alias = "extra-elektronikkas2005";
        }
        elsif ($cert =~ /Muntaner 244 Barcelona.*Firmaprofesional/)
        {
            $cert_alias = "extra-oldfirmaprofesional";
        }
        # Mozilla does not provide these certificates:
        #   baltimorecodesigningca
        #   gtecybertrust5ca
        #   trustcenterclass2caii
        #   trustcenterclass4caii
        #   trustcenteruniversalcai
        else
        {
            # Generate an alias using the OU and CN attributes of the
            # Subject field if both are present, otherwise use only the
            # CN attribute.  The Subject field must have either the OU
            # or the CN attribute.
            $_ = $cert;
            if ($cert =~ /OU=/)
            {
                s/Subject:.*?OU=//;
                # Remove other occurrences of OU=.
                s/OU=.*CN=//;
                # Remove CN= if there were not other occurrences of OU=.
                s/CN=//;
                s/\/emailAddress.*//;
                s/Certificate Authority/ca/g;
                s/Certification Authority/ca/g;
            }
            elsif ($cert =~ /CN=/)
            {
                s/Subject:.*CN=//;
                s/\/emailAddress.*//;
                s/Certificate Authority/ca/g;
                s/Certification Authority/ca/g;
            }
            s/\W//g;
            tr/A-Z/a-z/;
            $cert_alias = "extra-$_";
        }
        print "$cert => alias $cert_alias\n";
    }
    elsif ($cert =~ "Signature Algorithm: ecdsa")
    {
        # Ignore ECC certs since keytool rejects them
        $write_current_cert = 0;
        print " => ignoring ECC certificate\n";
    }
    elsif ($cert eq "-----BEGIN CERTIFICATE-----\n")
    {
        if ($in_cert_block != 0)
        {
            die "FAIL: $file is malformed.";
        }
        $in_cert_block = 1;
        if ($write_current_cert == 1)
        {
            $pem_file_count++;
            if (!sysopen(PEM, "$cert_alias.pem", O_WRONLY|O_CREAT|O_EXCL)) {
                $cert_alias = "$cert_alias.1";
                sysopen(PEM, "$cert_alias.1.pem", O_WRONLY|O_CREAT|O_EXCL)
                    || die("FAIL: could not open file for $cert_alias.pem: $!");
            }
            print PEM $cert;
            print " => writing $cert_alias.pem...\n";
        }
    }
    elsif ($cert eq "-----END CERTIFICATE-----\n")
    {
        $in_cert_block = 0;
        if ($write_current_cert == 1)
        {
            print PEM $cert;
            close(PEM);
        }
        $write_current_cert = 1
    }
    else
    {
        if ($in_cert_block == 1 && $write_current_cert == 1)
        {
            print PEM $cert;
        }
    }
}

# Check that the correct number of .pem files were produced.
@pem_files = <*.pem>;
if (@pem_files != $pem_file_count)
{
    print "$pem_file_count != ".@pem_files."\n";
    die "FAIL: Number of .pem files produced does not match".
        " number of certs read from $file.";
}

# Now store each cert in the 'cacerts' file using keytool.
$certs_written_count = 0;
foreach $pem_file (@pem_files)
{
    print "+ Adding $pem_file...\n";
    if (system("$ARGV[0] -import".
               " -alias `basename $pem_file .pem`".
               " -keystore cacerts -noprompt -storepass 'changeit' -file $pem_file") == 0) {
        $certs_written_count++;
    } else {
        print "FAILED\n";
    }
}

# Check that the correct number of certs were added to the keystore.
if ($certs_written_count != $pem_file_count)
{
    die "FAIL: Number of certs added to keystore does not match".
        " number of certs read from $file.";
}
