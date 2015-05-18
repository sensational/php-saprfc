<html>
<head>
   <title>SAPConnection Class: Connection to Logon Group</title>
</head>
<body>
<h1>SAPConnection Class: Connection to Logon Group</h1>
<?
    include_once ("../sap.php");

    $sap = new SAPConnection();
    // Params:                 message server R3 name Logon Group
    $sap->ConnectToLogonGroup("garfield",     "LNX",  "LNX");
    // Params:  client username  password  language
    $sap->Open ("900", "rfctest","zkouska","EN");
    $sap->PrintStatus();
    $sap->GetSystemInfo();
    echo "<BR><PRE>"; print_r ($sap); echo ("</PRE>");
    $sap->Close();



?>
</body>
</html>
