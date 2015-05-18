<?php
   /*        File : sap_table.php
    * Description : Class SAPTable
    *      Author : Eduard Koucky <eduard.koucky@czech-tv.cz>
    *      Source : http://saprfc.sourceforge.net/download
    *    Revision : $Id: sap_table.php,v 1.3 2002/05/17 07:13:53 koucky Exp $
    */

    /**
     *  Class SAPTable,
     */
    class SAPTable extends SAP {
    /*
     * PUBLIC VARS
     */
      var $fce = false;                  // Function handle
      var $name = "";                    // Table name
      var $row = array();                // Row
      var $rowNum = 0;                   // Counter
      var $rowStruct = array();          // Structure
      var $rowLast = 0;

      /**
       * Constructor
       */
      function SAPTable ($fce, $name)
      {
         parent::SAP();

         $this->fce = $fce;
         $this->name = $name;
         if ( $this->fce ) {
             $def = @saprfc_function_interface ($this->fce);
             if (is_array ($def)) {
                $ix = 0;
                while ( $ix < count ($def) &&
                      !( $def[$ix][name] == $name && $def[$ix][type] == "TABLE") )
                   $ix++;
                if ( $ix != count ($def) ) {
                   $members = &$def[$ix][def];
                   for ($j=0; $j<count ($members); $j++)
                      $this->rowStruct[] = $members[$j][name];
                }
             }
             $this->Rows();
         }
      }

      /**
       * Get number rows of internal table
       */
      function Rows() {
         if ( $this->__CheckParams () )
             $this->rowNum = @saprfc_table_rows ($this->fce, $this->name);
         else
             $this->rowNum = 0;
         return $this->rowNum;
      }

      /**
       * Init internal table
       */
      function Init() {
         if ($this->__CheckParams() )
             return @saprfc_table_init ($this->fce,$this->name);
      }

      /**
       * Append row into internal table
       */
      function Append ($row) {
         if ($this->__CheckParams() == false ) return false;
         unset ($this->row);
         $this->row = array();
         foreach ( $this->rowStruct as $member ) {
            if ( is_array($row) && isset ($row[$member]) )
               $this->row[$member] = $row[$member];
            else
               $this->row[$member] = "";
         }
         return  @saprfc_table_append ($this->fce, $this->name, $this->row);
      }

      /**
       * Insert raw into internal table
       */
      function Insert ($row, $index=1 ) {
         if ($this->__CheckParams($index,true,true) == false ) return false;
         unset ($this->row);
         $this->row = array();
         foreach ( $this->rowStruct as $member ) {
            if ( is_array($row) && isset ($row[$member]) )
               $this->row[$member] = $row[$member];
            else
               $this->row[$member] = "";
         }
         return @saprfc_table_insert ($this->fce, $this->name, $this->row, $index);
      }

      /**
       * Modify raw in internal table
       */
      function Modify ($row, $index=1 ) {
         if ($this->__CheckParams($index,true,true) == false ) return false;
         unset ($this->row);
         $this->row = @saprfc_read_table ($this->fce, $this->name, $index );
         foreach ( $this->rowStruct as $member ) {
            if ( is_array($row) && isset ($row[$member]) )
               $this->row[$member] = $row[$member];
         }
         return @saprfc_table_modify ($this->fce, $this->name, $this->row, $index);
      }

      /**
       * Delete raw in internal table
       */
      function Delete ($index=1 ) {
         if ($this->__CheckParams($index,true,true) == false ) return false;
         return @saprfc_table_remove ($this->fce, $this->name, $index);
      }

      /**
       * Read raw from internal table
       */
      function Read ($index=1 ) {
         if ($this->__CheckParams($index, true,true) == false ) return array();
         unset ($this->row);
         $this->row = @saprfc_table_read ($this->fce, $this->name, $index );
         return ($this->row);
      }

      /**
       * Get cell from table
       */
      function GetCell ($index, $col) {
         $this->Read ($index);
         return ($this->row[strtoupper ($col)]);
      }

      /**
       * Set cell in table
       */
      function SetCell ($index, $col, $value) {
         if ($this->__CheckParams($index, true,false) == false ) return false;
         $col = strtoupper ($col);
         if (! in_array ($col,$this->rowStruct) ) return false;
         if ($index > $this->rowNum )
           for ( $i=$this->rowNum; $i<=$index; $i++ )
             $this->Append (array());
         $this->Rows();
         $rc = $this->Modify (array ("$col"=>$value),$index);
         return ($rc);
      }

      /**
       *  Close internal table to array
       */
      function Close () {
          $this->fce = false;
          $this->name = "";
          $this->row = array();
          $this->rowNum = 0;
          $this->rowStruct = array();
      }


      /**
       *  Export internal table to array
       */
      function Export ($bottom=0, $top=0) {
          if ( $bottom < 1 ) $bottom = 1;
          if ( $top < 1 ) $top = $this->Rows();
          $retval = array();
          for ($i=$bottom; $i<=$top; $i++)
             $retval[] = $this->Read ($i);
          return ($retval);
      }

      /**
       *  Import array to internal table
       */
      function Import ($data) {
          $this->Init();
	  if (is_array ($data)) {
	    foreach ($data as $key => $value)
             $this->Append ($value);
	  }     
      }

      function Reset () {
          $this->rowLast = 1;
          $this->Rows();
      }

      function Next () {
          if ( $this->rowLast > $this->rowNum ) return false;
          unset ($this->row);
          $this->row = @saprfc_table_read ($this->fce, $this->name, $this->rowLast);
          $this->rowLast++;
          return true;
      }



      /*
       * PRIVATE METHODS
       */

      function __CheckParams ($index=1, $mincheck=false,$maxcheck=false) {
         $rc = $this->fce && $this->name != "";
         if ($mincheck) $rc = $rc && ($index >= 1);
         if ($maxcheck) $rc = $rc && ($index <= $this->Rows());
         return ($rc);
      }

    } // end of class SAP Table



?>
