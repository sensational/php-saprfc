<html>
<head>
   <title>SAPConnection Class: Connection to Application Server, Other Example</title>
</head>
<body>
<h1>SAPConnection Class: Connection to Application Server, Other Example</h1>
<?
    include_once ("../sap.php");

    $sap = new SAPConnection();
    $key = $sap-> GenerateEncryptKey ();
    $password = "zkouska";
    $encrypted = $sap->Encrypt($key,$password);
    $decrypted = $sap->Decrypt($key,$encrypted);
    echo ("Password: $password, Encrypted password: $encrypted, Decrypted password: $decrypted<BR>");
    // Params:                        hostname   sysnr
    $sap->ConnectToApplicationServer ("garfield", "30");
    // Params:  client username  password  language
    $sap->Open ("900", "rfctest",$decrypted,"EN");

     // Enable trace mode
    $sap->EnableTrace();
    // Set client code page
    $sap->SetCodePage ("1404");
    $sap->PrintStatus();
    $sap->GetSystemInfo();
    echo "<BR><PRE>"; print_r ($sap); echo ("</PRE>");
    echo "<H5>Attributes:</H5>";
    $attr = $sap->GetAttributes();
    echo "<BR><PRE>"; print_r ($attr); echo ("</PRE>");
    echo "R/3 NAME = ".$sap->GetR3Name()."<BR>";
    echo "R/3 RELEASE = ".$sap->GetR3Release()."<BR>";

    $sap->Close();


?>
</body>
</html>
