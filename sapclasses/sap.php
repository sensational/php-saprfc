<?php
   /*        File : sap.php
    * Description : An other class interface for saprfc PHP extension
    *      Author : Eduard Koucky <eduard.koucky@czech-tv.cz>
    *      Source : http://saprfc.sourceforge.net/download
    *    Revision : $Id: sap.php,v 1.1 2002/01/09 15:35:04 koucky Exp $
    */

    if (!defined('SAPCLASSES_DIR')) define('SAPCLASSES_DIR',dirname(__FILE__));

    // Define status values
    if (!defined("SAPRFC_OK"))         define("SAPRFC_OK",0);
    if (!defined("SAPRFC_ERROR"))      define("SAPRFC_ERROR",1);
    if (!defined("SAPRFC_APPL_ERROR")) define("SAPRFC_APPL_ERROR",99);

    /**
     *  Abstract SAP class,
     *  define status handling for derived classes
     */
    class SAP {
    /*
     * PUBLIC VARS
     */
      var $status = SAPRFC_OK;        // The status of last method executed
                                      // SAPRFC_OK (0) means success
      var $statusInfos = "";          // More status info (messages, BAPI struct)
      var $debug = false;             // Debug output

      /**
       * Constructor
       */
      function SAP()
      {
         if ( $this->CheckExtension() == false )
             die('SAPRFC extension is not installed. Exiting....');
      }

      /**
       * Set status, show errors if debug set
       *
       * @param [status]            Status code
       * @param [status_infos]      Status info structure
       *
       * @return status
       */
      function SetStatus($status,$status_infos) {
         $this->status=$status;
         $this->statusInfos=$status_infos;
         if ($this->debug &&
             $this->status!=SAPRFC_OK ) {
             $this->printStatus();
         }
         return $this->status;
      }
    
      /**
       * Get status of the last call of the method
       */
      function GetStatus() {
         return $this->status;
      }

      /**
       * Get text for Status/Error
       */
      function GetStatusText() {
         $statustext="";
         switch ($this->status) {
            case SAPRFC_OK:
                $statustext=$this->statusInfos;
                break;
            case SAPRFC_APPL_ERROR:
                $statustext=$this->statusInfos["TYPE"]." ".$this->statusInfos["ID"]."-".$this->statusInfos["NUMBER"].": ".$this->statusInfos["MESSAGE"];
                break;
            default:
                $statustext=$this->statusInfos;
                break;
         }
         return $statustext;
      }

      /**
       * Get long text for Status/Error
       */
      function getStatusTextLong() {
         $statustext="";
         switch ($this->status) {
            case SAPRFC_OK:
                $statustext.="<br><font size=4 color=green><pre>";
                $statustext.="No errors detected.";
                $statustext.="</font><br><font size=3 color=green><pre>";
                $statustext.="<br><b>".$this->getStatusText()."</b>";
                $statustext.="</pre></font>";
                break;
            case SAPRFC_APPL_ERROR:
                $statustext.="<br><font size=4 color=red><pre>";
                $statustext.="Application-Errors found during BAPI-Calls:";
                $statustext.="</font><br><font size=3 color=red><pre>";
                $statustext.="<br><b>".$this->getStatusText()."</b>";
                $statustext.="</pre></font>";
                break;
            default:
                $statustext.="<br><font size=4 color=red><pre>";
                $statustext.="Errors found during saprfc calls:";
                $statustext.="</font><br><font size=3 color=red><pre>";
                $statustext.="<br><b>".$this->getStatusText()."</b>";
                $statustext.="</pre></font>";
		        break;
         }
         return $statustext;
      }

	  /**
	   * Print Status
       */
      function PrintStatus() {
		  echo $this->getStatusTextLong();
      }

	  /**
	   * Set on debug mode
       */
      function DebugOn() {
          $this->debug = true;
      }
      
	  /**
	   * Set off debug mode
       */
      function DebugOff() {
          $this->debug = false;
      }
      
	  /**
	   * Check SAPRFC extension
	   *
       * @return true if saprfc extension is loaded
       */
       function CheckExtension() {
          return (extension_loaded ("saprfc"));
       }


    } // end of class SAP

    // include other SAP classes
    include_once(SAPCLASSES_DIR."/sap_connection.php");     // class SAPConnection
    include_once(SAPCLASSES_DIR."/sap_function.php");       // class SAPFunction
    include_once(SAPCLASSES_DIR."/sap_table.php");          // class SAPTable


?>
