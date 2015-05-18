<html>
<head>
   <title>SAPConnection Class: Connection to Application Server</title>
</head>
<body>
<h1>SAPConnection Class: Connection to Application Server</h1>
<?
    include_once ("../sap.php");

    $sap = new SAPConnection();
    // Params:                        hostname   sysnr
    $sap->ConnectToApplicationServer ("garfield", "30");
    // Params:  client username  password  language
    $sap->Open ("900", "rfctest","zkouska","EN");
    $sap->PrintStatus();
    $sap->GetSystemInfo();
    echo "<BR><PRE>"; print_r ($sap); echo ("</PRE>");
    $sap->Close();
?>
</body>
</html>
