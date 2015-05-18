<html>
<head>
   <title>SAPFunction Class: Get Report</title>
</head>
<body>
<h1>SAPFunction Class: Get Report</h1>
<?
// ----------------------------------------------------------------------------
// SAPRFC - Client example
// Call function module RFC_READ_REPORT - Get ABAP report from SAP R/3
// http://saprfc.sourceforge.net
// ----------------------------------------------------------------------------
//----------------------------------------------------------------------
//RFC_READ_REPORT:
//       IMPORTING
//             VALUE(PROGRAM) LIKE  SY-REPID
//       EXPORTING
//             VALUE(SYSTEM) LIKE  SY-SYSID
//             VALUE(TRDIR) LIKE  TRDIR STRUCTURE  TRDIR
//       TABLES
//              QTAB STRUCTURE  D022S
//----------------------------------------------------------------------


$REPORT = "RSUSR000";                            // Set name of the report

// ----------------------------------------------------------------------------

    include_once("../sap.php");

    $sap = new SAPConnection();
    $sap->Connect("logon_data.conf");
    if ($sap->GetStatus() == SAPRFC_OK ) $sap->Open ();
    if ($sap->GetStatus() != SAPRFC_OK ) {
       $sap->PrintStatus();
       exit;
    }

    $fce = &$sap->NewFunction ("RFC_READ_REPORT");
    if ($fce == false ) {
       $sap->PrintStatus();
       exit;
    }

   $fce->PROGRAM = $REPORT;
   $fce->Call();
   switch ($fce->GetStatus) {
      case SAPRFC_OK:  echo "<PRE><B>SYSTEM:</B>\n"; print_r ($fce->SYSTEM) ; echo "</PRE>";
                       echo "<PRE><B>TRDIR:</B>\n"; print_r ($fce->TRDIR) ; echo "</PRE>";
                       $fce->QTAB->Reset();
                       while ($fce->QTAB->Next())
                          echo $fce->QTAB->row[LINE]."<BR>\n";
                       break;
      case SAPRFC_EXCEPTION:
                       echo "Exception: ".$fce->GetException();
                       break;
      default:
                       $fce->PrintStatus();
                       break;
   }
   $sap->Close();
?>
</body>
</html>
