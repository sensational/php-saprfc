<html>
<h1>SAPRFC-Class: Get List of Users in SAP-System</h1>
<?
	// Example for using the saprfc-class-library for accessing sap-functions via rfc
	// provided by lars laegner, btexx business technologies, august 2001
	//  !!!! PLEASE CHANGE THE LOGINDATA TO YOUR SAP-SYSTEM !!!!
	
	// $Id: example_userlist.php,v 1.2 2001/08/16 15:54:35 llaegner Exp $
	
	// saprfc-class-library	
	require_once("saprfc.php");
	
	// Create saprfc-instance
	$sap = new saprfc(array(
						"logindata"=>array(
							"ASHOST"=>"yourhost"		// application server
							,"SYSNR"=>"00"				// system number
							,"CLIENT"=>"800"			// client
							,"USER"=>"youruser"			// user
							,"PASSWD"=>"yourpwd"		// password
							)
						,"show_errors"=>false			// let class printout errors
						,"debug"=>false)) ; 				// detailed debugging information

	// Call-Function
	$result=$sap->callFunction("SO_USER_LIST_READ",
								array(	array("IMPORT","USER_GENERIC_NAME","*"),
										array("TABLE","USER_DISPLAY_TAB",array())
								));			 					
				
	// Call successfull?
	if ($sap->getStatus() == SAPRFC_OK) {
		// Yes, print out the Userlist
		?><table>
			<tr><td>SAP-Name</td><td>User-Nummer</td></tr><?
			
		foreach ($result["USER_DISPLAY_TAB"] as $user) {
			echo "<tr><td>", $user["SAPNAM"],"</td><td>",$user["USRNO"],"</td></tr>";
		}
		
		?></table><?
	} else { 
		// No, print long Version of last Error
		$sap->printStatus();
		// or print your own error-message with the strings received from
		// 		$sap->getStatusText() or $sap->getStatusTextLong()
	}
	
	// Logoff/Close saprfc-connection LL/2001-08
	$sap->logoff();
?>
