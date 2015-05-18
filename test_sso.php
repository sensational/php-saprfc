<?php
# Test SAP Single Sign On (SSO2) 
# $Id: test_sso.php,v 1.1 2005/12/18 16:26:23 koucky Exp $
# Prerequisite:
#    Profile parameters: login/create_sso2_ticket = 1
#                        login/accept_sso2_ticket = 1
#    Transaction SSO2 - Administration logon ticket 
#        


# The first logon to SAP using username and password, request for ticket GETOSS2 = 1
# Please change logon parameters for your system

$login = array (
			"ASHOST"=>"<hostname>",
			"SYSNR"=>"00",
			"CLIENT"=>"<client>",
			"USER"=>"<username>",
			"PASSWD"=>"<password>",
      "GETSSO2"=>"1");


$rfc = saprfc_open ($login );
if (! $rfc ) { echo "RFC connection failed"; exit; }


# Get SSO ticket
$ticket = saprfc_get_ticket($rfc);

saprfc_close ($rfc);

if ($ticket)
   echo "SSO2 ticket = ".$ticket. "<br>";
  

# The second logon to SAP using SSO2 ticket 
unset ($login["PASSWD"]);
unset ($login["USER"]);
unset ($login["GETSS02"]);
$login["MYSAPSSO2"] = $ticket;

$rfc = saprfc_open ($login );
if (! $rfc ) { echo "SSO2 RFC connection failed"; exit; }

echo "SSO2 OK<br>";

saprfc_close ($rfc);

?>
