<html>
<head>
   <title>SAPFunction Class: TRFC Call  STFC_WRITE_TO_TCPIC</title>
</head>
<body>
<h1>SAPFunction Class: TRFC Call  STFC_WRITE_TO_TCPIC</h1>
<?
    include_once("../sap.php");
	
    $sap = new SAPConnection();
    $sap->Connect("logon_data.conf");
    if ($sap->GetStatus() == SAPRFC_OK ) $sap->Open ();
    if ($sap->GetStatus() != SAPRFC_OK ) {
       $sap->PrintStatus();
       exit;
    }

    $fce = &$sap->NewFunction ("STFC_WRITE_TO_TCPIC");
    if ($fce == false ) {
       $sap->PrintStatus();
       exit;
    }

    $fce->RESTART_QNAME = "";
    $fce->TCPICDAT->Append (array ("LINE"=>"line1"));
    $fce->TCPICDAT->Append (array ("LINE"=>"line2"));
    $fce->TCPICDAT->Append (array ("LINE"=>"line3"));
    $tid = $fce->GetTID();
    $fce->IndirectCall($tid);

    if ($fce->GetStatus() == SAPRFC_OK)
        echo "TID=$tid, OK";
    else
    {
        $fce->PrintStatus();
        echo "Call again with TID = $tid";
    }

	$sap->Close();
?>
</body>
</html>
