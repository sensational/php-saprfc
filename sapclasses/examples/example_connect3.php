<html>
<head>
   <title>SAPConnection Class: Connection From User Defined Config File</title>
</head>
<body>
<h1>SAPConnection Class: Connection From User Defined Config File</h1>
<?
    include_once ("../sap.php");

    $sap = new SAPConnection();
    // Params:                 message server R3 name Logon Group
    $sap->Connect("logon_data.conf");
    if ($sap->GetStatus() == SAPRFC_OK )
       $sap->Open ();
    $sap->PrintStatus();
    $sap->GetSystemInfo();
    echo "<BR><PRE>"; print_r ($sap); echo ("</PRE>");
    $sap->Close();



?>
</body>
</html>
