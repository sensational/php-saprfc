<html>
<head>
   <title>SAPFunction Class: Get List of Users in SAP-System</title>
</head>
<body>
<h1>SAPFunction Class: Get List of Users in SAP-System</h1>
<?
    include_once("../sap.php");
	
    $sap = new SAPConnection();
    $sap->Connect("logon_data.conf");
    if ($sap->GetStatus() == SAPRFC_OK ) $sap->Open ();
    if ($sap->GetStatus() != SAPRFC_OK ) {
       $sap->PrintStatus();
       exit;
    }

    $fce = &$sap->NewFunction ("SO_USER_LIST_READ");
    if ($fce == false ) {
       $sap->PrintStatus();
       exit;
    }

    $fce->USER_GENERIC_NAME = "*";
    $fce->Call();
    // $fce->Debug();

    if ($fce->GetStatus() == SAPRFC_OK) {
        echo "<table><tr><td>SAP-Name</td><td>User-Number</td></tr>";
        $fce->USER_DISPLAY_TAB->Reset();
        while ( $fce->USER_DISPLAY_TAB->Next() )
            echo "<tr><td>".$fce->USER_DISPLAY_TAB->row["SAPNAM"]."</td><td>".$fce->USER_DISPLAY_TAB->row["USRNO"]."</td></tr>";
        echo "</table>";
    } else
        $fce->PrintStatus();

	$sap->Close();
?>
</body>
</html>
