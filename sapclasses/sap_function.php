<?php
   /*        File : sap_function.php
    * Description : Class SAPFunction
    *      Author : Eduard Koucky <eduard.koucky@czech-tv.cz>
    *      Source : http://saprfc.sourceforge.net/download
    *    Revision : $Id: sap_function.php,v 1.4 2002/07/09 08:07:44 koucky Exp $
    */

    /**
     *  Class SAPFunction,
     */
    class SAPFunction extends SAP {
    /*
     * PUBLIC VARS
     */
      var $rfc = false;                  // RFC handle
      var $fce = false;                  // Function handle
      var $name = "";                    // Function name
      var $exception = "";               // Exception from last call
      var $server = false;               // true if it is server function
      var $bapi = false;                 // BAPI function
      var $def = false;                  // Interface definition

      /**
       * Constructor
       */
      function SAPFunction($fce=false)
      {
         parent::SAP();

         if ($fce)
         {
            $this->rfc = false;
            $this->fce = $fce;
            $this->name = @saprfc_function_name($this->fce);
            $this->exception = "";
            $this->server = true;
            $this->bapi = substr ($this->name,0,4) == "BAPI";
            $this->def = $this->GetDefinition ();
            $this->InitVars();
         }
      }

      /**
       * Discover interface of function module
       *
       * @param [rfc]            RFC handle
       * @param [name]           Name of function module
       *
       * @return SAPRFC_OK on success, SAPRFC_ERROR on failure
       */
      function Discover ($rfc,$name) {
         if ( $this->fce )
             return $this->SetStatus (SAPRFC_ERROR,"SAPFunction::Discover: Function is ".
                                                    "already defined.");
         $this->fce = @saprfc_function_discover ($rfc,strtoupper($name));
         if ( $this->fce == false )
             return $this->SetStatus (SAPRFC_ERROR,"SAPFunction::Discover: ".
                                      "Failed for function module ".$name."\n".
                                      "with error ".@saprfc_error());
         $this->rfc = $rfc;
         $this->name = @saprfc_function_name ($this->fce);
         $this->exception = "";
         $this->server = false;
         $this->bapi = substr ($this->name,0,4) == "BAPI";
         $this->def = $this->GetDefinition ();
         $this->InitVars();
         return $this->SetStatus (SAPRFC_OK,"");
      }

      /**
       * Define interface of function module
       *
       * @param [rfc]            RFC handle
       * @param [name]           Name of function module
       * @param [def]            Definition of function interface
       *
       * @return SAPRFC_OK on success, SAPRFC_ERROR on failure
       */
      function Define ($rfc,$name,$def) {
         if ( $this->fce )
             return $this->SetStatus (SAPRFC_ERROR,"SAPFunction::Define: Function is ".
                                                    "already defined.");
         $this->fce = @saprfc_function_define ($rfc,strtoupper($name),$def);
         if ( $this->fce == false )
             return $this->SetStatus (SAPRFC_ERROR,"SAPFunction::Define: ".
                                      "Failed for function module ".$name."\n".
                                      "with error ".@saprfc_error());
         $this->rfc = $rfc;
         $this->name = @saprfc_function_name ($this->fce);
         $this->exception = "";
         $this->server = ( $rfc == false  ) ? true : false;
         $this->bapi = substr ($this->name,0,4) == "BAPI";
         $this->def = $this->GetDefinition ();
         $this->InitVars();
         return $this->SetStatus (SAPRFC_OK,"");
      }

      /**
       * Get definition of discovered function module
       *
       * @return array as definition
       */
      function GetDefinition () {
         if ( $this->fce == false )
            $def = false;
         else
             $def = @saprfc_function_interface ($this->fce);
         return ($def);
      }

      /**
       * Show debug info
       *
       */
      function Debug () {
         if ( $this->fce)
             @saprfc_function_debug_info ($this->fce);
      }

      /**
       * Get text definition of discovered function module
       *
       * @return php code define interface
       */
      function GetStringDefinition () {
         if ($this->def == false) return ("array()");

         $buf = "array (\n";
         for ( $i=0; $i<count($this->def); $i++ )
         {
            $interface = $this->def[$i];
            $buf.= "  \t array (\n";
            $buf.= "  \t\t \"name\"=>\"".$interface[name]."\",\n";
            $buf.= "  \t\t \"type\"=>\"".$interface[type]."\",\n";
            $buf.= "  \t\t \"optional\"=>\"".$interface[optional]."\",\n";
            $buf.= "  \t\t \"def\"=> array (\n";
            $typedef = $interface[def];
            for ( $j=0; $j<count($typedef); $j++ )
            {
               $buf.= "  \t\t\t array (\"name\"=>\"".$typedef[$j][name].
                      "\",\"abap\"=>\"".$typedef[$j][abap]."\",\"len\"=>".
                      $typedef[$j][len].",\"dec\"=>".$typedef[$j][dec].
		      ",\"offset\"=>".$typedef[$j][offset].")";
               if ($j < (count($typedef)-1) )  $buf.= ",\n";
                                         else  $buf.= "\n";
            }
            $buf.= "  \t\t\t)\n";
            $buf.= "  \t)";
            if ($i < (count($def)-1) ) $buf.= ",\n";
                                  else $buf.= "\n";
         }
         $buf.= "); \n";
         return ($buf);
      }

      /**
       * Get name of function module
       *
       */
      function GetName () {
          return $this->name;
      }

      /**
       * Get exception from last call
       *
       */
      function GetException () {
          return $this->exception;
      }

      /**
       * Init class vars
       *
       */
      function InitVars ($shutdown=false) {
         if ($this->def == false ) return;
         for ( $i=0; $i<count($this->def); $i++ )
         {
             $interface = &$this->def[$i];
             $name = strtoupper($interface[name]);
             $type = $interface[type];
             if (  isset ($this->$name) )
             {
                 if ($type == "TABLE" && is_object ($this->$name) )
                 {
                    if ($shutdown == false)
                      $this->$name->Init();
                    else
                    {
                      $this->$name->Close();
                      unset ($this->$name);
                    }
                 }
                 else
                    unset ($this->$name);
             }
             elseif ( $type == "TABLE" && $shutdown == false)
                 $this->$name = new SAPTable ($this->fce,$name);
         }
      }

      /**
       * Copy class variables to the interface function module
       *
       */
      function ExportVars () {
         if ($this->def == false ) return;
         for ( $i=0; $i<count($this->def); $i++ )
         {
             $interface = &$this->def[$i];
             $name = strtoupper($interface[name]);
             $type = $interface[type];
             $members = &$interface[def];
             if ( $type != "TABLE" && $members[0][name] != "")
                 $type.="_STRUCT";
             switch ($type) {
                case 'IMPORT'  :
                      if ($this->server == false)
                      {
                          if ( isset ($this->$name) )
                             $rc = @saprfc_import ($this->fce,$name,$this->$name);
                          else
                             $rc = @saprfc_import ($this->fce,$name,"");
                          assert ($rc);
                      }
                      break;
                case 'IMPORT_STRUCT' :
                     if ($this->server == false)
                     {
                         $vars = array();
                         for ($j=0; $j < count ($members); $j++)
                         {
                            $mname = $members[$j][name];
			    $mstruct = &$this->$name;
                            if (isset ($this->$name) && isset ($mstruct[$mname]) )
                               $vars[$mname] = $mstruct[$mname];
                            else
                               $vars[$mname] = "";
                         }
                         $rc = @saprfc_import ($this->fce,$name,$vars);
                         assert ($rc);
                         unset ($vars);
                     }
                     break;
                case 'EXPORT'  :
                      if ($this->server == true)
                      {
                          if ( isset ($this->$name) )
                             $rc = @saprfc_server_export ($this->fce,$name,$this->$name);
                          else
                             $rc = @saprfc_server_export ($this->fce,$name,"");
                          assert ($rc);
                      }
                      break;
                case 'EXPORT_STRUCT' :
                     if ($this->server == true)
                     {
                         $vars = array();
                         for ($j=0; $j < count ($members); $j++)
                         {
                            $mname = $members[$j][name];
			    $mstruct = &$this->$name;
                            if (isset ($this->$name) && isset ($mstruct[$mname]) )
                               $vars[$mname] = $mstruct[$mname];
                            else
                               $vars[$mname] = "";
                         }
                         $rc = @saprfc_server_export ($this->fce,$name,$vars);
                         assert ($rc);
                         unset ($vars);
                     }
                     break;
                case 'TABLE' :
                     if (!isset ($this->$name) || !is_object ($this->$name) )
                         @saprfc_table_init ($this->fce,$name);
                     break;
             }  //switch
         } //for
      }

      /**
       * Copy interface function module to class vars
       *
       */
      function ImportVars () {
         if ($this->def == false ) return;
         for ( $i=0; $i<count($this->def); $i++ )
         {
             $interface = &$this->def[$i];
             $name = strtoupper($interface[name]);
             $type = $interface[type];
             switch ($type) {
                case 'IMPORT'  :
                      if ($this->server == true)
                      {
                          if ( isset ($this->$name) ) unset ($this->$name);
                          $this->$name = @saprfc_server_import ($this->fce,$name);
                      }
                      break;
                case 'EXPORT'  :
                      if ($this->server == false)
                      {
                          if ( isset ($this->$name) ) unset ($this->$name);
                          $this->$name = @saprfc_export ($this->fce,$name);
                      }
                      break;
                case 'TABLE' :
                     if (!isset ($this->$name) || !is_object ($this->$name) )
                          $this->$name = new SAPTable ($this->fce,$name);
                     break;
             }  //switch
         } //for
      }

      /**
       * Call function module
       *
       * @return SAPRFC_OK on success, SAPRFC_EXECPTION if exception raised
       */
      function Call () {
         if ( $this->fce == false )
             return $this->SetStatus (SAPRFC_ERROR,"SAPFunction::Call: Function is ".
                                      "not defined.");
         $this->ExportVars();
         $rc = @saprfc_call_and_receive ($this->fce);
         if ($rc == SAPRFC_OK)
            $this->ImportVars();
         elseif ($rc == SAPRFC_EXCEPTION)
            $this->exception = @saprfc_exception ($this->fce);

         if ($this->bapi) {
			$bapi_return=$this->RETURN;
			if (isset($bapi_return) &&
                    is_array($bapi_return) &&
                    isset($bapi_return["MESSAGE"]) &&
                    $bapi_return["NUMBER"] != 0) {
                return $this->setStatus(SAPRFC_APPL_ERROR,$bapi_return);
	        }
         }
         return $this->SetStatus ($rc,@saprfc_error());
      }

      /**
       * Get Transaction ID
       *
       * @return Transaction ID
       */
      function GetTID () {
         if ( $this->rfc == false ) return "";
         return (@saprfc_trfc_tid($this->rfc));
      }

      /**
       * Call function module indirectly, TRFC
       *
       * @param [tid]            Transaction ID
       *
       * @return SAPRFC_OK on success
       */
      function IndirectCall ($tid) {
         if ( $this->fce == false )
             return $this->SetStatus (SAPRFC_ERROR,"SAPFunction::IndirectCall: Function is ".
                                      "not defined.");
         $this->ExportVars();
         $rc = @saprfc_trfc_call ($this->fce,$tid);
         $this->exception = "";
         return $this->SetStatus ($rc,@saprfc_error());
      }

      /**
       * Close function module free, resources
       *
       */
       function Close () {
         if ($this->def)
         {
             $this->InitVars(true);
             unset ($this->def);
         }
         if ( $this->server == false && $this->fce )
             @saprfc_function_free ($this->fce);
         $this->fce = false;
         $this->rfc = false;
         $this->name = "";
       }


    } // end of class SAP Table



?>
