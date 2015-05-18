<?php
   /*        File : sap_connection.php
    * Description : Class SAPConnetion
    *      Author : Eduard Koucky <eduard.koucky@czech-tv.cz>
    *      Source : http://saprfc.sourceforge.net/download
    *    Revision : $Id: sap_connection.php,v 1.2 2002/05/17 07:14:37 koucky Exp $
    */

     // The type of SAP connection
     define ("SAPRFC_CONN_NONE",           0); // no connection
     define ("SAPRFC_CONN_APPSERVER",      1); // client connection to the single
                                               // application server
     define ("SAPRFC_CONN_LOGONGROUP",     2); // client load balanced connection
                                               // to the logon group
     define ("SAPRFC_CONN_USERDEFINED",    3); // client user defined
     define ("SAPRFC_CONN_ACCEPT",         4); // server connection

    /**
     *  Class SAPConnection,
     */
    class SAPConnection extends SAP {
    /*
     * PUBLIC VARS
     */
      var $rfc = false;                  // RFC handle for connection
      var $connType = SAPRFC_CONN_NONE;  // Connection type
      var $appServer = "";               // Hostname of the application server
      var $systemNumber = "";            // System number, 00 - default
      var $gatewayHost = "";             // Hostname of the SAP Gateway
      var $gatewayService = "";          // Service name of the SAP Gateway (sapgwXX)
      var $messageServer = "";           // Hostname of the message server for Load Balancing
      var $R3SystemName = "";            // Name (<SID>) of R/3 System
      var $logonGroup = "";              // Name of the logon group
      var $userLogonData = false;        // User defined logon data
      var $trace = false;                // RFC trace
      var $codePage = "";                // SAP Codepage for connection, empty = default
      var $serverReg = false;            // Registration parameters for RFC server
      var $systemInfo = false;           // System information about Remote SAP system
      var $fceList = array();            // Function list
    /*
     * PUBLIC METHODS
           ConnectToApplicationServer($ashost,$sysnr="00")
           ConnectToLogonGroup($mshost,$r3name,$group)
           Connect($connect_data)
           Open ($client="",$user="",$passwd="",$lang="")
           Accept ($progid="",$gwhost="",$gwserv="")
           Close ()
           EnableTrace ()
           DisableTrace ()
           SetCodePage ($codepage)
           GetSystemInfo ()
           GetAttributes ()
           GetHandle ()
           GetR3Name ()
           GetR3Release ()
           IsR3AtLeast40A ()
           IsR3AtLeast46A ()
           IsConnectionLive ()
           GenerateEncryptKey ()
           Encrypt ($key, $passwd)
           Decrypt ($key, $encrypted)
           NewFunction ($name)
     */

      /**
       * Constructor
       */
      function SAPConnection()
      {
         parent::SAP();
      }

      /**
       * Define connection to the application server
       *
       * @param [ashost]            Hostname of the application server
       * @param [sysnr]             System Number
       *
       * @return SAPRFC_OK on success, SAPRFC_ERROR on failure
       */
      function ConnectToApplicationServer($ashost,$sysnr="00") {
         if ( $this->rfc ) $this->Close();
         $this->connType = SAPRFC_CONN_APPSERVER;
         $this->appServer = $ashost;
         $this->systemNumber = $sysnr;
         return $this->SetStatus (SAPRFC_OK,"");
      }

      /**
       * Define connection to the logon group (load balanced)
       *
       * @param [mshost]            Hostname of the message server
       * @param [r3name]            Name of R3 System
       * @param [group]             Name of logon group
       *
       * @return SAPRFC_OK on success, SAPRFC_ERROR on failure
       */
      function ConnectToLogonGroup($mshost,$r3name,$group) {
         if ( $this->rfc ) $this->Close();
         $this->connType = SAPRFC_CONN_LOGONGROUP;
         $this->messageServer = $mshost;
         $this->R3SystemName = $r3name;
         $this->logonGroup = $group;
         return $this->SetStatus (SAPRFC_OK,"");
      }

      /**
       * User defined connection
       *
       * @param [connect_data]        Logon data in saprfc_open() format
       *                              or filename with logon data
       *
       * @return SAPRFC_OK on success, SAPRFC_ERROR on failure
       */
      function Connect($connect_data) {
         if ( $this->rfc ) $this->Close();
         if ( is_array ($connect_data) )
         {
            $this->connType = SAPRFC_CONN_USERDEFINED;
            $this->userLogonData = $connect_data;
            $this->SetStatus (SAPRFC_OK,"");
         }
         else
         {
            $lines = @file ($connect_data);
            if (! is_array ($lines) )
            {
               $this->userLogonData = false;
               return $this->SetStatus (SAPRFC_ERROR,"SAPConnection::Connect: Fail open connect file ".
                                              $connect_data);
            }
            $this->connType = SAPRFC_CONN_USERDEFINED;
            for ( $i=0; $i < count ($lines); $i++ )
            {
                $l = trim ($lines[$i]);
                if ( substr ($l,0,1) == '#' ) continue;  // comments
                $p = explode ("=",$l,2);
                if ( is_array ($p) && count ($p) == 2 )
                    $this->userLogonData[trim(strtoupper($p[0]))] = $p[1];
                unset ($p);
            }
            $this->SetStatus (SAPRFC_OK,"");
         }
         return $this->GetStatus ();
      }

      /**
       * Open RFC client connection to SAP R/3 System
       *
       * @param [client]        Client number
       * @param [user]          User name
       * @param [passwd]        Password
       * @param [language]      Language (empty = default)
       *
       * @return SAPRFC_OK on success, <> SAPRFC_OK on failure
       */
      function Open ($client="",$user="",$passwd="",$lang="") {
         if ( $this->rfc )
             return $this->SetStatus (SAPRFC_ERROR,"SAPConnection::Open: Connection is ".
                                                    "already opened, close it first.");

         switch ($this->connType) {
            case SAPRFC_CONN_NONE :
                    return $this->SetStatus (SAPRFC_ERROR,"SAPConnection::Open: Connection is ".
                                                          "is not defined.");
            case SAPRFC_CONN_APPSERVER :
                    $connect_data[ASHOST] = $this->appServer;
                    $connect_data[SYSNR] = $this->systemNumber;
                    break;

            case SAPRFC_CONN_LOGONGROUP :
                    $connect_data[MSHOST] = $this->messageServer;
                    $connect_data[R3NAME] = $this->R3SystemName;
                    $connect_data[GROUP] = $this->logonGroup;
                    break;

            case SAPRFC_CONN_USERDEFINED :
                    $connect_data = $this->userLogonData;
                    break;
            default :
                    return $this->SetStatus (SAPRFC_ERROR,"SAPConnection::Open: Unsupported connection type");

         }

         if ( !empty ($client) ) $connect_data[CLIENT] = $client;
         if ( !empty ($user)   ) $connect_data[USER]  = $user;
         if ( !empty ($passwd) ) $connect_data[PASSWD]  = $passwd;
         if ( !empty ($lang)   ) $connect_data[LANG]  = $lang;

         $this->rfc = @saprfc_open ($connect_data);
         if ( $this->rfc == false )
              return $this->SetStatus (SAPRFC_ERROR,@saprfc_error());

         if ($this->trace) $this->EnableTrace();
                     else  $this->DisableTrace();

         if (! empty ($this->codePage) ) $this->SetCodePage ($this->codePage);

         return ( $this->SetStatus (SAPRFC_OK,"") );

      }

      /**
       * Accept incoming RFC connection (for RFC server)
       *
       * @param [progid]         PROGRAM ID
       * @param [gwhost]         SAP gateway host
       * @param [gwserv]         SAP gateway service
       *                         if parameters is no set, $argv
       *                         is used
       *
       * @return SAPRFC_OK on success, <> SAPRFC_OK on failure
       */
      function Accept ($progid="",$gwhost="",$gwserv="") {
         GLOBAL $argv;

         if ( $this->rfc ) $this->Close();
         if ( empty ($progid) && empty ($gwhost) && empty ($gwserv) )
         {
             for ($i=0; $i<count ($argv); $i++ )
             {
                 switch ($argv[$i]) {
                      case '-a' :  $progid = $argv[$i+1]; break;
                      case '-g' :  $gwhost = $argv[$i+1]; break;
                      case '-x' :  $gwserv = $argv[$i+1]; break;
                 }
             }
             $this->rfc = @saprfc_server_accept ($argv);
         }
         else
             $this->rfc = @saprfc_server_accept ("-a $progid -g $gwhost -x $gwserv");

         if ( $this->rfc == false )
              return $this->SetStatus (SAPRFC_ERROR,@saprfc_error());

         $this->serverReg = array ("progid"=>$progid, "gwhost"=>$gwhost, "gwserv"=>$gwserv );

         if ($this->trace) $this->EnableTrace();
                     else  $this->DisableTrace();

         if (! empty ($this->codePage) ) $this->SetCodePage ($this->codePage);

         return ( $this->SetStatus (SAPRFC_OK,"") );
      }

      /**
       * Close RFC connection to SAP R/3 System
       *
       */
       function Close () {
          $this->connType = SAPRFC_CONN_NONE;
          if (is_array ($this->userLogonData)) unset ($this->userLogonData);
          $this->userLogonData = false;
          if (is_array ($this->serverReg)) unset ($this->serverReg);
          $this->serverReg = false;
          if (is_array ($this->systemInfo)) unset ($this->systemInfo);
          $this->systemInfo = false;
          for ($i=0; $i<count ($this->fceList); $i++ )
          {
              $this->fceList[$i]->Close();
              unset ($this->fceList[$i]);
          }
          if (is_array ($this->fceList)) unset ($this->fceList);
          $this->fceList = array();
          $this->__InitConnectData();
          if ( $this->rfc )
          {
              @saprfc_close ($this->rfc);
              $this->rfc = false;
          }
       }

      /**
       * Enable RFC trace
       *
       */
      function EnableTrace () {
         $this->trace = true;
         if ($this->rfc)
             @saprfc_set_trace ($this->rfc,$this->trace);
      }

      /**
       * Disable RFC trace (default)
       *
       */
      function DisableTrace () {
         $this->trace = false;
         if ($this->rfc)
             @saprfc_set_trace ($this->rfc,$this->trace);
      }

      /**
       * Set SAP codepage for connection
       *
       * @param [codepage]       SAP codepage number
       *
       */
      function SetCodePage ($codepage) {
         $this->codePage = $codepage;
         if ($this->rfc)
             @saprfc_set_code_page ($this->rfc,$codepage);
      }

      /**
       * Get system info about remote SAP system
       *
       * @return array[APPHOST] ......... application server
       *         array[APPOS] ........... operating system
       *         array[APPIP] ........... IP address
       *         array[APPDEST].......... RFC destination
       *         array[SAPSYS] .......... SAP system id
       *         array[SAPREL] .......... SAP R/3 release version
       *         array[SAPCP] ........... SAP R/3 code page
       *         array[SAPKREL] ......... SAP R/3 Kernel release version
       *         array[DBNAME] .......... Database name
       *         array[DBHOST] .......... Database host
       *         array[DBTYPE] .......... Database system (Oracle...)
       *
       */
       function GetSystemInfo () {
          if ($this->rfc == false ) return array();
          if ( is_array ($this->systemInfo) )
             return $this->systemInfo;

          $sysinfo = array();
          $fce = @saprfc_function_discover ($this->rfc, "RFC_SYSTEM_INFO");
          if ($fce)
          {
             $rc = @saprfc_call_and_receive ($fce);
             if ( $rc == SAPRFC_OK )
                $sysinfo = @saprfc_export ($fce,"RFCSI_EXPORT");
             @saprfc_function_free ($fce);
          }
          $attr = $this->GetAttributes ();
          $this->systemInfo["APPHOST"]
                 = $sysinfo["RFCHOST"] != "" ? $sysinfo["RFCHOST"] : $attr["partner_host"];
          $this->systemInfo["APPOS"]
                 = $sysinfo["RFCOPSYS"];
          $this->systemInfo["APPIP"]
                 = $sysinfo["RFCIPADDR"] != "" ? $sysinfo["RFCIPADDR"] :
                                                 gethostbyname ($this->systemInfo["APPHOST"]);
          $this->systemInfo["SAPSYS"]
                 = $sysinfo["RFCSYSID"] != "" ? $sysinfo["RFCSYSID"] : $attr["sysid"];
          $this->systemInfo["SAPREL"]
                 = $sysinfo["RFCSAPRL"] != "" ? $sysinfo["RFCSAPRL"] : $attr["partner_rel"];
          $this->systemInfo["SAPCP"]
                 = $sysinfo["RFCCHARTYP"] != "" ? $sysinfo["RFCCHARTYP"] : $attr["partner_codepage"];
          $this->systemInfo["SAPKREL"]
                 = $sysinfo["RFCKERNRL"] != "" ? $sysinfo["RFCKERNRL"] : $attr["kernel_rel"];
          $this->systemInfo["DBNAME"]
                 = $sysinfo["RFCDATABS"] != "" ? $sysinfo["RFCDATABS"] :
                                                 $this->systemInfo["SAPSYS"];
          $this->systemInfo["DBHOST"]
                 = $sysinfo["RFCDBHOST"];
          $this->systemInfo["DBTYPE"]
                 = $sysinfo["RFCDBSYS"];
          $this->systemInfo["APPDEST"]
                 = $sysinfo["RFCDEST"] != "" ? $sysinfo["RFCDEST"] :
                                               $this->systemInfo["APPHOST"]."_".
                                               $this->systemInfo["SAPSYS"]."_".
                                               $attr["systnr"];
          unset ($sysinfo); unset ($attr);
          return $this->systemInfo;
       }

      /**
       * Get Attributes (see saprfc_attributes)
       *
       */
      function GetAttributes () {
         if ($this->rfc) return @saprfc_attributes($this->rfc);
                    else return array();
      }

      /**
       * Get RFC handle
       *
       */
      function GetHandle () {
          return $this->rfc;
      }

      /**
       * Get SAP R/3 release version
       *
       */
      function GetR3Release () {
          $this->GetSystemInfo();
          return $this->systemInfo["SAPREL"];
      }

      /**
       * Get SAP R/3 System name
       *
       */
      function GetR3Name () {
          $this->GetSystemInfo();
          return $this->systemInfo["SAPSYS"];
      }


      /**
       * Determines if the application server is running Release 4.0A or
       * a later version of the R/3 System
       *
       */
      function IsR3AtLeast40A () {
          $rel = $this->GetR3Release();
          if ( empty ($rel) ) return false;
          $major = substr ($rel,0,1);
          return ( intval($major) >= 4 ) ? true : false;
      }


      /**
       * Determines if the application server is running Release 4.6A or
       * a later version of the R/3 System
       *
       */
      function IsR3AtLeast46A () {
          $rel = $this->GetR3Release();
          if ( empty ($rel) ) return false;
          $major = substr ($rel,0,1);
          $minor = substr ($rel,0,1);
          if ( ( intval($major) > 4 ) ||
             ( ( intval($major) == 4 ) && ( intval ($minor) >= 6 ) ) )
             return true;
          else
             return false;
      }

      /**
       * Check if RFC connection is valid
       *
       * @return SAPRFC_OK on success, <> SAPRFC_OK on failure
       */
      function IsConnectionLive () {
         if ( $this->rfc == false )
             return $this->SetStatus (SAPRFC_ERROR,"SAPConnection::IsConnectionLive: No valid RFC handle");

         $fce = @saprfc_function_discover($this->rfc,"RFC_PING");
         if ( $fce == false )
              return $this->SetStatus (SAPRFC_ERROR,@saprfc_error());

         $rc = @saprfc_call_and_receive ($fce);
         @saprfc_function_free ($fce);
         if ( $rc == SAPRFC_OK ) return $this->SetStatus (SAPRFC_OK,"");
                            else return $this->SetStatus (SAPRFC_ERROR,@saprfc_error());
      }

      /**
       * Genereate key for password encryption
       *
       * @return key
       */
      function GenerateEncryptKey () {
           return substr (md5 (uniqid (rand())),0,24);
      }

      /**
       * Encrypt password string
       * require mcrypt extension
       *
       * @param [key]            encrypt key
       * @param [passwd]         clean text password
       *
       * @return encrypted password
       */
      function Encrypt ($key, $passwd) {
         if ( $this->__CheckMcryptExtension () == true )
         {
            $td = @mcrypt_module_open (MCRYPT_TripleDES, "", MCRYPT_MODE_ECB, "");
            $iv = @mcrypt_create_iv (mcrypt_enc_get_iv_size ($td), MCRYPT_RAND);
            @mcrypt_generic_init ($td, $key, $iv);
            $encrypted = base64_encode(@mcrypt_generic ($td, $passwd));
            @mcrypt_generic_end ($td);
            return $encrypted;
         }
         else return $passwd;
      }

      /**
       * Decrypt password string
       * require mcrypt extension
       *
       * @param [key]            encrypt key
       * @param [encrypted]      encrypted password
       *
       * @return clean text password
       */
      function Decrypt ($key, $encrypted) {
         if ( $this->__CheckMcryptExtension () == true )
         {
            $td = @mcrypt_module_open (MCRYPT_TripleDES, "", MCRYPT_MODE_ECB, "");
            $iv = @mcrypt_create_iv (mcrypt_enc_get_iv_size ($td), MCRYPT_RAND);
            @mcrypt_generic_init ($td, $key, $iv);
            $passwd = @mdecrypt_generic ($td, base64_decode ($encrypted));
            @mcrypt_generic_end ($td);
            return $passwd;
         }
         else return $encrypted;
      }

      /**
       * Get SAPFunction object
       *
       * @param [name]           function name
       *
       * @return SAPFunction object or false
       */
      function NewFunction ($name)
      {
          $ix = count ($this->fceList);
          $this->fceList[$ix] = new SAPFunction();
          $rc = $this->fceList[$ix]->Discover ($this->rfc,strtoupper($name));
          if ($rc != SAPRFC_OK) {
              $this->fceList[$ix]->Close();
              unset ($this->fceList[$ix]);
              return (false);
          }
          return ($this->fceList[$ix]);
      }


      /*
       * PRIVATE METHODS
       */

      function __InitConnectData () {
         $this->appServer = "";
         $this->systemNumber = "";
         $this->gatewayHost = "";
         $this->gatewayService = "";
         $this->messageServer = "";
         $this->R3SystemName = "";
         $this->logonGroup = "";
         $this->trace = false;
         $this->codePage = "";
      }

      function __CheckMcryptExtension () {
          if ( ! extension_loaded ("mcrypt") ) return false;
          if ( function_exists ('mcrypt_module_open') &&
               function_exists ('mcrypt_create_iv') &&
               function_exists ('mcrypt_enc_get_iv_size') &&
               function_exists ('mcrypt_generic_init') &&
               function_exists ('mcrypt_generic') &&
               function_exists ('mdecrypt_generic') &&
               function_exists ('mcrypt_generic_end') &&
               function_exists ('mcrypt_generic_init') )
               return true;
          else
               return false;
      }


    } // end of class SAP Connection



?>
