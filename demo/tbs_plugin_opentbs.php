<?php

/* clsOpenTBS, by Skrol29
version 1.1, on 2009-11-19
This class can open a zip file, read the central directory, and 
 retrieve the content of a zipped file which is not compressed.
Site: http://www.tinybutstrong.com/plugins.php
clsOpenTBS is under the LGPL license.
*/

// Constants to drive the plugin.
define('OPENTBS_PLUGIN','clsOpenTBS');
define('OPENTBS_DOWNLOAD',1);   // download (default) = TBS_OUTPUT
define('OPENTBS_NOHEADER',4);   // option to use with DOWNLOAD: no header is sent
define('OPENTBS_FILE',8);       // output to file
define('OPENTBS_DEBUG_XML',16); // display the result of the current subfile
define('OPENTBS_STRING',32);    // output to string
define('OPENTBS_INFO',1);       // command to display the archive info
define('OPENTBS_RESET',2);      // command to reset the changes in the current archive

// default parameters corresponding to file extensions known by OpenTBS
global $_OPENTBS_AutoExt, $_OPENTBS_Meth8Ok;
if (!isset($_OPENTBS_AutoExt)) {
	$_OPENTBS_AutoExt = array();
	$_OPENTBS_AutoExt['odt'] = array('content.xml', '<text:line-break/>', 'application/vnd.oasis.opendocument.text', '&apos;', '\'');
	$_OPENTBS_AutoExt['odg'] = array('content.xml', '<text:line-break/>', 'application/vnd.oasis.opendocument.graphics', '&apos;', '\'');
	$_OPENTBS_AutoExt['ods'] = array('content.xml', '<text:line-break/>', 'application/vnd.oasis.opendocument.spreadsheet', '&apos;', '\'');
	$_OPENTBS_AutoExt['odf'] = array('content.xml', false, 'application/vnd.oasis.opendocument.formula', '&apos;', '\'');
	$_OPENTBS_AutoExt['odp'] = array('content.xml', '<text:line-break/>', 'application/vnd.oasis.opendocument.presentation', '&apos;', '\'');
	//$_OPENTBS_AutoExt['odb'] = array('content.xml', false, 'application/vnd.sun.xml.base');
	$x = array(chr(226).chr(128).chr(152) , chr(226).chr(128).chr(153));
	$_OPENTBS_AutoExt['docx'] = array('word/document.xml', '<w:br/>', 'application/vnd.openxmlformats-officedocument.wordprocessingml.document',$x,'\'');
	$_OPENTBS_AutoExt['xlsx'] = array('xl/worksheets/sheet1.xml;xl/sharedStrings.xml', false, 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
	$_OPENTBS_AutoExt['pptx'] = array('ppt/slides/slide1.xml', false, 'application/vnd.openxmlformats-officedocument.presentationml.presentation',$x,'\'');
	unset($x);
}

class clsOpenTBS extends clsMiniZip {

	function OnInstall() {
		$TBS =& $this->TBS;
		$TBS->OtbsAutoLoad = true; // TBS will load the subfile regarding to the extension of the archive
		$TBS->OtbsConvBr = false;  // string for NewLine conversion
		$TBS->OtbsAutoUncompress = true;
		$this->Version = '1.1'; // Version can be displayed using [onshow..tbs_info] since TBS 3.2.0
		$this->Meth8Ok = extension_loaded('zlib');
		return array('BeforeLoadTemplate','BeforeShow', 'OnCommand');
	}
	
	function BeforeLoadTemplate(&$File,&$HtmlCharSet) {

		$TBS =& $this->TBS;
		
		if ($TBS->_Mode!=0) return; // If we are in subtemplate mode, the we use the TBS default process

		// Decompose the file path. The syntaxe is 'Archive.ext#subfile', or 'Archive.ext', or '#subfile'
		$p = strpos($File, '#');
		if ($p===false) {
			$FilePath = $File;
			$SubFileLst = false;
		} else {
			$FilePath = substr($File,0,$p);
			$SubFileLst = substr($File,$p+1);
		}

		// Open the archive
		if ($FilePath!=='') {
			$this->Open($FilePath); // Open the archive
			if ($TBS->OtbsAutoLoad and ($this->ArchExtInfo!==false) and ($SubFileLst===false)) {
				$SubFileLst = $this->ArchExtInfo[0];
				$TBS->OtbsConvBr = $this->ArchExtInfo[1];
			}
			$TBS->LoadTemplate('', array(&$this,'ConvXML')); // Define the function for string conversion
			$TBS->OtbsCurrFile = false;
			$TBS->OtbsSubFileLst = $SubFileLst;
			$this->TbsSrcParking = array();
		} elseif ($this->ArchFile==='') {
			$this->RaiseError('Cannot read file(s) "'.$SubFileLst.'" because no archive is opened.');
		}

		// Load the subfile(s)
		if (($SubFileLst!=='') and ($SubFileLst!==false)) {
			
			if (is_string($SubFileLst)) $SubFileLst = explode(';',$SubFileLst);
			
			$ModeSave = $TBS->_Mode; 
			$TBS->_Mode++;    // deactivate TplVars[] reset and Charset reset.
			$TBS->Plugin(-4); // deactivate other plugins 
			
			foreach ($SubFileLst as $SubFile) {
				
				$idx = $this->FileGetIdx($SubFile);
				if ($idx===false) {
					$this->RaiseError('The file "'.$SubFile.'" is not found in the archive "'.$this->ArchFile.'".');
				} else {
					
					// Check if the subfile can be uncompress
					if (!isset($this->CdFileLst[$idx]['_uncomp'])) {
						$this->CdFileLst[$idx]['_uncomp'] = false; // tells if the file is uncompressed or not
						if ($TBS->OtbsAutoUncompress) {
							$meth = $this->CdFileLst[$idx]['meth'];
							if ($meth==8) {
								if ($this->Meth8Ok) {
									$this->CdFileLst[$idx]['_deflate'] = true;
									$this->CdFileLst[$idx]['_uncomp'] = true;
								} else {
									$this->RaiseError('The file "'.$SubFile.'" is compressed in the archive "'.$this->ArchFile.'" with the ZIP method 8. OpenTBS cannot uncompress this file because the Zlib extension is not activated in the PHP configuration. You should activate the Zlib extension or uncompress or see the manual tp know how to change the compression method in the archive.');
								}
							} elseif ($meth==0) {
								$this->CdFileLst[$idx]['_uncomp'] = true;
							} else {
								$this->RaiseError('The file "'.$SubFile.'" is compressed in the archive "'.$this->ArchFile.'" with the ZIP method '.$meth.'. OpenTBS is not able to uncompress this file. See the manual to know how to change the compression method in the archive.');
							}
						}
					}
					
					// Save the current loaded subfile if any
					$this->TbsSrcPark();
					
					// load the subfile
					if (isset($this->TbsSrcParking[$idx])) {
						$TBS->Source = $this->TbsSrcParking[$idx]; // Load from parking
					} else {
						$TBS->Source = $this->FileRead($idx); // Load from the archive
						if (isset($this->CdFileLst[$idx]['_deflate'])) $TBS->Source = gzinflate($TBS->Source);
						if (($this->ArchExtInfo!==false) and isset($this->ArchExtInfo[3])) {
							$TBS->Source = str_replace($this->ArchExtInfo[3],$this->ArchExtInfo[4],$TBS->Source);
						}
					}
					
					// apply default TBS behaviors on the uncompressed content: other plug-ins + [onload] fields  
					if ($this->CdFileLst[$idx]['_uncomp']) $TBS->LoadTemplate(null,'+');
					
					$TBS->OtbsCurrFile = $SubFile;
					
				}
				
			}
			
			// Reactivate default configuration
			$TBS->_Mode = $ModeSave;
			$TBS->Plugin(-10); // reactivate other plugins
			
		}
		
		if ($FilePath!=='') $TBS->_LastFile = $FilePath;
		
		return false; // default LoadTemplate() process is not executed
		
	}

	function BeforeShow(&$Render, $File='') {
		
		$TBS =& $this->TBS;

		if ($TBS->_Mode!=0) return; // If we are in subtemplate mode, the we use the TBS default process
		
		if (($Render & OPENTBS_DEBUG_XML)==OPENTBS_DEBUG_XML) {
			$TBS->Plugin(-4); // deactivate other plugins
			$TBS->Show();
			exit;
		}
		
		$this->TbsSrcPark(); // Save the current loaded subfile if any
		
		$TBS->Plugin(-4); // deactivate other plugins 
		
		// Merges all modified subfiles
		$idx_lst = array_keys($this->TbsSrcParking);
		foreach ($idx_lst as $idx) {
			$TBS->Source = $this->TbsSrcParking[$idx];
			unset($this->TbsSrcParking[$idx]); // save memory space
			$TBS->Show(TBS_NOTHING);
			$len_u = strlen($TBS->Source);
			$crc32 = crc32($TBS->Source);
			$meth = $this->CdFileLst[$idx]['meth'];
			if ($meth<>0) {
				if ($TBS->OtbsAutoUncompress) {
					if (isset($this->CdFileLst[$idx]['_deflate'])) {
						$TBS->Source = gzdeflate($TBS->Source);
					} else {
						$meth = 0;
					}
				} else {
					// recompress is managed manually by user
				}
			}
			$this->FileReplace($idx, $TBS->Source, $len_u, $meth, $crc32);
		}
		$TBS->Plugin(-10); // reactivate other plugins

		if (($TBS->ErrCount>0) and (!$TBS->NoErr)) {
			$TBS->meth_Misc_Alert('Show() Method', 'The output is cancelled by the OpenTBS plugin because at least one error has occured.');
			exit;
		}

		if (($Render & TBS_OUTPUT)==TBS_OUTPUT) {
			// download
			$this->Flush(OPENTBS_DOWNLOAD, $File);
			$Render = $Render - TBS_OUTPUT; //prevent TBS from an extra output.
		} elseif(($Render & OPENTBS_FILE)==OPENTBS_FILE) {
			// to file
			$this->Flush(OPENTBS_FILE, $File);
		} elseif(($Render & OPENTBS_STRING)==OPENTBS_STRING) {
			// to string
			$this->Flush(OPENTBS_STRING);
			$TBS->Source = $this->OutputSrc;
			$this->OutputSrc = '';
		}
		
		if (($Render & TBS_EXIT)==TBS_EXIT) {
			$this->Close();
			exit;
		}
		
		return false; // default Show() process is not executed
		
	}

	function OnCommand($cmd) {
		// Display debug information
		if ($cmd==OPENTBS_INFO) {
			echo "<strong>OpenTBS plugin Information</strong><br>\r\n";
			$this->Debug();	
		} elseif ($cmd==OPENTBS_RESET) {
			$this->ResetChanges();
			$this->TbsSrcParking = array();
			$TBS =& $this->TBS;
			$TBS->Source = '';
			$TBS->OtbsCurrFile = false;
			if (is_string($TBS->OtbsSubFileLst)) {
				$f = '#'.$TBS->OtbsSubFileLst;
				$h = '';
				$this->BeforeLoadTemplate($f,$h);
			}
		}
	}
	
	function TbsSrcPark() {
		// save the last opened subfile
		if ($this->LastReadIdx!==false) {
			$this->TbsSrcParking[$this->LastReadIdx] = $this->TBS->Source;
			$this->TBS->Source = '';
			$this->LastReadIdx = false;
		}
	}

	function ConvXml($Txt, $ConvBr) {
	// Used by TBS to convert special chars and new lines.
	  $x = htmlspecialchars(utf8_encode($Txt));
	  if ($ConvBr) {
	  	$z = $this->TBS->OtbsConvBr;
	  	if ($z!==false) {
		    $x = nl2br($x); // Convert any type of line break
		    $x = str_replace('<br />',$z ,$x);
		  }
	  }
	  return $x;
	}

	function RaiseError($Msg) {
		// Overwrite the parent RaiseError() method.
		$this->TBS->meth_Misc_Alert('OpenTBS Plugin', $Msg);
		if (!$this->TBS->NoErr) exit;
		return false;
	}

}

class clsMiniZip {
/* Version 1.0, by Skrol29, on 2009-11-03
MiniZip is a class which can read the structure of a simple Zip archive and replace some files (see requirements below).
- MiniZip doesn't support Zip64 archives.
- MiniZip doesn't support zip file comment (very rarely used, not editable in 7-Zip yet).
- MiniZip doesn't need any PHP extension.
- MiniZip can replace exiting file data in a Zip Archive, but it cannot
   uncompress of compress file. When you read a file of an archive, MiniZip will
   return the compressed data (if data are compressed). And it will save data
   compressed only if you compress it before. 
- MiniZip cannot add a delete a file in an archive yet. Only read and replace.
- MiniZip is under the LGPL license.
Site: http://www.tinybutstrong.com/plugins.php
*/
	
	function Open($ArchFile) {
	// Open the zip archive
		$this->Close(); // close handle and init info
		$this->Error = false;
		$this->ArchFile = $ArchFile;
		// Save Archive extension (to be used for auto load subfile
		$x = basename($ArchFile);
		$p = strrpos($x, '.');
		$x = ($p===false) ? ''  : strtolower(substr($x,$p+1));
		if (isset($GLOBALS['_OPENTBS_AutoExt'][$x])) $this->ArchExtInfo =& $GLOBALS['_OPENTBS_AutoExt'][$x];
		$this->ArchExt = $x;
		// open the file
		$this->ArchHnd = fopen($ArchFile, 'rb');
		$ok = !($this->ArchHnd===false);
		if ($ok) $ok = $this->CentralDirRead();
		return $ok;
	}
	
	function Close() {
		if (isset($this->ArchHnd) and ($this->ArchHnd!==false)) fclose($this->ArchHnd);
		$this->ArchFile = '';
		$this->ArchExt = '';
		unset($this->ArchExtInfo); // this value can be assigned by reference
		$this->ArchExtInfo = false;
		$this->ArchHnd = false;
		$this->CdInfo = array();
		$this->CdFileLst = array();
		$this->CdFileNbr = 0;
		$this->CdFileByName = array();
		$this->VisFileLst = array();
		$this->ResetChanges();
	}	

	function ResetChanges() {
		$this->LastReadIdx = false;
		$this->ReplInfo = array();
		$this->ReplByPos = array();
	}
	
	function CentralDirRead() {
		$cd_info = 'PK'.chr(05).chr(06); // signature of the Central Directory
		$cd_pos = -22;
		$this->_MoveTo($cd_pos, SEEK_END);
		$b = $this->_ReadData(4);
		if ($b!==$cd_info) return $this->RaiseError('The footer of the Central Directory is not found.');

		$this->CdEndPos = ftell($this->ArchHnd) - 4;
		$this->CdInfo = $this->CentralDirRead_End($cd_info);
		$this->CdFileLst = array();
		$this->CdFileNbr = $this->CdInfo['file_nbr_curr'];
		$this->CdPos = $this->CdInfo['p_cd'];

		if ($this->CdFileNbr<=0) return $this->RaiseError('No file found in the Central Directory.');
		if ($this->CdPos<=0) return $this->RaiseError('No position found for the Central Directory listing.');

		$this->_MoveTo($this->CdPos);
		for ($i=0;$i<$this->CdFileNbr;$i++) {
			$x = $this->CentralDirRead_File();
			if ($x!==false) {
				$this->CdFileLst[$i] = $x;
				$this->CdFileByName[$x['v_name']] = $i;
			}
		}
		return true;
	}

	function CentralDirRead_End($cd_info) {
		$b = $cd_info.$this->_ReadData(18);
		$x = array();
		$x['disk_num_curr'] = $this->_GetDec($b,4,2); // number of this disk
		$x['disk_num_cd'] = $this->_GetDec($b,6,2);   // number of the disk with the start of the central directory
		$x['file_nbr_curr'] = $this->_GetDec($b,8,2); // total number of entries in the central directory on this disk
		$x['file_nbr_tot'] = $this->_GetDec($b,10,2);  // total number of entries in the central directory
		$x['l_cd'] = $this->_GetDec($b,12,4);          // size of the central directory
		$x['p_cd'] = $this->_GetDec($b,16,4);         // offset of start of central directory with respect to the starting disk number
		$x['l_comm'] = $this->_GetDec($b,20,2);       // .ZIP file comment length
		$x['v_comm'] = $this->_ReadData($x['l_comm']); // .ZIP file comment
		$x['bin'] = $b.$x['v_comm'];
		return $x;
	}
	
	function CentralDirRead_File() {

		$b = $this->_ReadData(46);

		$x = $this->_GetHex($b,0,4);
		if ($x!=='h:02014b50') return $this->RaiseError('Signature of file information not found in the Central Directory.');

		$x = array();
		$x['vers_used'] = $this->_GetDec($b,4,2);
		$x['vers_necess'] = $this->_GetDec($b,6,2);
		$x['purp'] = $this->_GetBin($b,8,2);
		$x['meth'] = $this->_GetDec($b,10,2);
		$x['time'] = $this->_GetDec($b,12,2);
		$x['date'] = $this->_GetDec($b,14,2);
		$x['crc32'] = $this->_GetDec($b,16,4);
		$x['l_data_c'] = $this->_GetDec($b,20,4);
		$x['l_data_u'] = $this->_GetDec($b,24,4);
		$x['l_name'] = $this->_GetDec($b,28,2);
		$x['l_fields'] = $this->_GetDec($b,30,2);
		$x['l_comm'] = $this->_GetDec($b,32,2);
		$x['disk_num'] = $this->_GetDec($b,34,2);
		$x['int_file_att'] = $this->_GetDec($b,36,2);
		$x['ext_file_att'] = $this->_GetDec($b,38,4);
		$x['p_loc'] = $this->_GetDec($b,42,4);
		$x['v_name'] = $this->_ReadData($x['l_name']);
		$x['v_fields'] = $this->_ReadData($x['l_fields']);
		$x['v_comm'] = $this->_ReadData($x['l_comm']);

		$x['bin'] = $b.$x['v_name'].$x['v_fields'].$x['v_comm'];

		return $x;
	}

	function RaiseError($Msg) {
		$this->Error = $Msg;
		return false;
	}

	function Debug() {

		echo "<br />\r\n";

		echo "------------------<br />\r\n";
		echo "Archive Extension:<br />\r\n";
		echo "------------------<br />\r\n";
		print_r($this->ArchExtInfo);

		echo "<br />\r\n";

		echo "------------------<br />\r\n";
		echo "Central Directory:<br />\r\n";
		echo "------------------<br />\r\n";
		print_r($this->CdInfo);

		echo "<br />\r\n";

		echo "----------<br />\r\n";
		echo "File List:<br />\r\n";
		echo "----------<br />\r\n";
		print_r($this->CdFileLst);			
		
	}

	function FileExists($NameOrIdx) {
		return ($this->FileGetIdx($NameOrIdx)!==false);
	}
	
	function FileGetIdx($NameOrIdx) {
	// Check if a file name, or a file index exists in the Central Directory, and return its Index
		if (is_string($NameOrIdx)) {
			if (isset($this->CdFileByName[$NameOrIdx])) {
				return $this->CdFileByName[$NameOrIdx];
			} else {
				return false;
			}
		} else {
			if (isset($this->CdFileLst[$NameOrIdx])) {
				return $NameOrIdx;
			} else {
				return false;
			}
		}
	}
	
	function FileRead($NameOrIdx) {
		
		$idx = $this->FileGetIdx($NameOrIdx);
		if ($idx===false) return $this->RaiseError('File "'.$NameOrIdx.'" is not found in the Central Directory.');

		$pos = $this->CdFileLst[$idx]['p_loc'];
		$this->_MoveTo($pos);

		$this->LastReadIdx = $idx; // Can be usefull to get the idx

		return $this->_FileRead($idx, true);

	}

	function _FileRead($idx, $ReadData) {

		$b = $this->_ReadData(30);
		
		$x = $this->_GetHex($b,0,4);
		if ($x!=='h:04034b50') return $this->RaiseError('Signature of file information not found as expected in postion '.$pos.'.');

		$x = array();
		$x['vers'] = $this->_GetDec($b,4,2);
		$x['purp'] = $this->_GetBin($b,6,2);
		$x['meth'] = $this->_GetDec($b,8,2);
		$x['time'] = $this->_GetDec($b,10,2);
		$x['date'] = $this->_GetDec($b,12,2);
		$x['crc32'] = $this->_GetDec($b,14,4);
		$x['l_data_c'] = $this->_GetDec($b,18,4);
		$x['l_data_u'] = $this->_GetDec($b,22,4);
		$x['l_name'] = $this->_GetDec($b,26,2);
		$x['l_fields'] = $this->_GetDec($b,28,2);
		$x['v_name'] = $this->_ReadData($x['l_name']);
		$x['v_fields'] = $this->_ReadData($x['l_fields']);

		$x['bin'] = $b.$x['v_name'].$x['v_fields'];

		// Read Data
		$len_cd = $this->CdFileLst[$idx]['l_data_c'];
		if ($x['l_data_c']==0) {
			// Sometimes, the size is not specified in the local information.
			$len = $len_cd;
		} else {
			$len = $x['l_data_c'];
			if ($len!=$len_cd) {
				//echo "MiniZip Warning: Local information for file #".$idx." says len=".$len.", while Central Directory says len=".$len_cd.".";
			}
		}

		if ($ReadData) {
			$Data = $this->_ReadData($len);
		} else {
			$this->_MoveTo($len, SEEK_CUR);
		}
		
		// Description information
		$desc_ok = ($x['purp'][2+3]=='1');
		if ($desc_ok) {
			$b = $this->_ReadData(16);
			$x['desc_bin'] = $b;
			$x['desc_sign'] = $this->_GetHex($b,0,4); // not specified in the documentation sign=h:08074b50
			$x['desc_crc32'] = $this->_GetDec($b,4,4);
			$x['desc_l_data_c'] = $this->_GetDec($b,8,4);
			$x['desc_l_data_u'] = $this->_GetDec($b,12,4);
		}

		// Save file info without the data
		$this->VisFileLst[$idx] = $x;

		// Return the info
		if ($ReadData) {
			return $Data;
		} else {
			return true;
		}
		
	}

	function FileReplace($NameOrIdx, $Data, $len_u = false, $meth = false, $crc32 = false) {
	// Store replacement data and information

		$idx = $this->FileGetIdx($NameOrIdx);
		if ($idx===false) return $this->RaiseError('File "'.$NameOrIdx.'" is not found in the Central Directory.');

		$pos = $this->CdFileLst[$idx]['p_loc'];
		if ($Data===false) {
			unset($this->ReplByPos[$pos]);
			unset($this->ReplInfo[$idx]);
		} else {
			$this->ReplByPos[$pos] = $idx;
			$this->ReplInfo[$idx] = array();
			$ReplInfo =& $this->ReplInfo[$idx];
			$ReplInfo['data'] = $Data;
			$ReplInfo['len_c'] = strlen($Data);
			$ReplInfo['len_u'] = ($len_u===false) ? $ReplInfo['len_c'] : $len_u;
			$ReplInfo['delta'] = $ReplInfo['len_c'] - $this->CdFileLst[$idx]['l_data_c'];
			if ($crc32===false) { // The crc32 must be calculated on uncompress data
				$ReplInfo['crc32'] = crc32($Data);
			} else {
				$ReplInfo['crc32'] = $crc32;
			}
			$ReplInfo['meth'] = $meth;
		}
	}

	function Flush($Render=OPENTBS_DOWNLOAD, $File='', $ContentType='') {

		$ArchPos = 0;
		$Delta = 0;
		$FicNewPos = array();

		$this->OutputOpen($Render, $File, $ContentType);
		
		// output modified zipped files
		ksort($this->ReplByPos);
		foreach ($this->ReplByPos as $ReplPos => $ReplIdx) {
			// output data from the zip archive which is before the data to replace
			$this->OutputFromFile($ArchPos, $ReplPos);
			// get info of the zipped file
			if (!isset($this->VisFileLst[$ReplIdx])) $this->_FileRead($ReplIdx, false);
			$FileInfo =& $this->VisFileLst[$ReplIdx];
			// get replace info
			$ReplInfo =& $this->ReplInfo[$ReplIdx];
			$ReplInfo['p_loc'] = $ReplPos + $Delta;
			// prepare the header of the zipped file
			$b1 = $FileInfo['bin'];
			$this->_PutDec($b1, $ReplInfo['crc32'], 14, 4); // crc32
			$this->_PutDec($b1, $ReplInfo['len_c'], 18, 4); // l_data_c
			$this->_PutDec($b1, $ReplInfo['len_u'], 22, 4); // l_data_u
			if ($ReplInfo['meth']!==false) $this->_PutDec($b1, $ReplInfo['meth'], 8, 2); // meth
			// prepare the bottom description if the zipped file, if any
			if (isset($FileInfo['desc_bin'])) {
				$b2 = $FileInfo['desc_bin'];
				$this->_PutDec($b2, $ReplInfo['crc32'], 4, 4);  // crc32
				$this->_PutDec($b2, $ReplInfo['len_c'], 8, 4);  // l_data_c
				$this->_PutDec($b2, $ReplInfo['len_u'], 12, 4); // l_data_u
			} else {
				$b2 = '';
			}
			// output data
			$this->OutputFromString($b1.$ReplInfo['data'].$b2);
			// Update the delta of positions for subfiles which are physically after the currently replaced one
			$Delta = $Delta + $ReplInfo['delta'];
			for ($i=0;$i<$this->CdFileNbr;$i++) {
				if ($this->CdFileLst[$i]['p_loc']>$ReplPos) {
					$FicNewPos[$i] = $this->CdFileLst[$i]['p_loc'] + $Delta;
				}
			}
			// Update the current pos in the archive
			$ArchPos = $ReplPos + strlen($b1) + $this->CdFileLst[$ReplIdx]['l_data_c'] + strlen($b2); // $FileInfo['l_data_c'] may have a 0 value in some archives
			//echo "ArchPos=$ArchPos , Delta=$Delta, header1=".strlen($b1)." , header2=".strlen($b2);
		}
		
		// Ouput all the files of to the Central Directory listing
		$this->OutputFromFile($ArchPos, $this->CdPos);
		$ArchPos = $this->CdPos;
		
		$b2 = '';
		for ($i=0;$i<$this->CdFileNbr;$i++) {
			$b1 = $this->CdFileLst[$i]['bin'];
			if (isset($FicNewPos[$i])) $this->_PutDec($b1, $FicNewPos[$i], 42, 4);   // p_loc
			if (isset($this->ReplInfo[$i])) {
				$ReplInfo =& $this->ReplInfo[$i];
				$this->_PutDec($b1, $ReplInfo['crc32'], 16, 4); // crc32
				$this->_PutDec($b1, $ReplInfo['len_c'], 20, 4);   // l_data_c
				$this->_PutDec($b1, $ReplInfo['len_u'], 24, 4); // l_data_u
				if ($ReplInfo['meth']!==false) $this->_PutDec($b1, $ReplInfo['meth'], 10, 2); // meth
			}
			$b2 .= $b1;
		}
		$this->OutputFromString($b2);
		$ArchPos += strlen($b2);
		
		// Output until Central Directory footer
		$this->OutputFromFile($ArchPos, $this->CdEndPos);

		// Output Central Directory footer
		$b2 = $this->CdInfo['bin'];
		$this->_PutDec($b2, $this->CdPos+$Delta , 16, 4); // p_cd
		$this->OutputFromString($b2);
		
	}

	// ----------------
	// output functions
	// ----------------

	function OutputOpen($Render, $File, $ContentType) {

		if (($Render & OPENTBS_FILE)==OPENTBS_FILE) {
			if (''.$File=='') $File = basename($this->ArchFile).'.zip';
			$this->OutputHandle = fopen($File, 'w');
			$this->OutputMode = OPENTBS_FILE;
		} elseif (($Render & OPENTBS_STRING)==OPENTBS_STRING) {
			$this->OutputMode = OPENTBS_STRING;
			$this->OutputSrc = '';
		} elseif (($Render & OPENTBS_DOWNLOAD)==OPENTBS_DOWNLOAD) {
			$this->OutputMode = OPENTBS_DOWNLOAD;
			// Calculate the new lenght of the file
			$Len = filesize($this->ArchFile);
			foreach ($this->ReplByPos as $i) {
				$Len += $this->ReplInfo[$i]['delta'];
			}
			// Output the file
			if (''.$File=='') $File = basename($this->ArchFile);
			if (($ContentType=='') and ($this->ArchExtInfo!==false)) $ContentType = $this->ArchExtInfo[2];
			if (($Render & OPENTBS_NOHEADER)==OPENTBS_NOHEADER) {
			} else {
				header ('Pragma: no-cache');
				if ($ContentType!='') header ('Content-Type: '.$ContentType);
				header('Content-Disposition: attachment; filename="'.$File.'"');
				header('Expires: 0');
				header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
				header('Cache-Control: public');
				header('Content-Description: File Transfer'); 
				header('Content-Transfer-Encoding: binary');
				if ($Len!==false) header('Content-Length: '.$Len); 
			}
		}
	}

	function OutputFromFile($pos, $pos_stop) {
		$len = $pos_stop - $pos;
		if ($len<0) return;
		$this->_MoveTo($pos);
		$block = 1024;
		while ($len>0) {
			$l = min($len, $block);
			$x = $this->_ReadData($l);
			$this->OutputFromString($x);
			$len = $len - $l;
		}
		unset($x);
	}

	function OutputFromString($data) {
		if ($this->OutputMode===OPENTBS_DOWNLOAD) {
			echo $data; // donwload
		} elseif ($this->OutputMode===OPENTBS_STRING) {
			$this->OutputSrc .= $data; // to string
		} elseif (OPENTBS_FILE) {
			fwrite($this->OutputHandle, $data); // to file
		}
	}

	function OutputClose() {
		if ($this->OutputHandle!==false) fclose($this->OutputHandle);
	}

	// ----------------
	// Reading functions
	// ----------------

	function _MoveTo($pos, $relative = SEEK_SET) {
		fseek($this->ArchHnd, $pos, $relative);
	}

	function _ReadData($len) {
		if ($len>0) {
			$x = fread($this->ArchHnd, $len);
			return $x;
		} else {
			return '';
		}
	}

	// ----------------
	// Take info from binary data
	// ----------------

	function _GetDec($txt, $pos, $len) {
		$x = substr($txt, $pos, $len);
		$z = 0;
		for ($i=0;$i<$len;$i++) {
			$asc = ord($x[$i]);
			if ($asc>0) $z = $z + $asc*pow(256,$i);
		}
		return $z;
	}

	function _GetHex($txt, $pos, $len) {
		$x = substr($txt, $pos, $len);
		return 'h:'.bin2hex(strrev($x));
	}

	function _GetBin($txt, $pos, $len) {
		$x = substr($txt, $pos, $len);
		$z = '';
		for ($i=0;$i<$len;$i++) {
			$asc = ord($x[$i]);
			if (isset($x[$i])) {
				for ($j=0;$j<8;$j++) {
					$z .= ($asc & pow(2,$j)) ? '1' : '0';
				}
			} else {
				$z .= '00000000';
			}
		}
		return 'b:'.$z;
	}

	// ----------------
	// Put info into binary data
	// ----------------

	function _PutDec(&$txt, $val, $pos, $len) {
		$x = '';
		for ($i=0;$i<$len;$i++) {
			if ($val==0) {
				$z = 0;
			} else {
				$z = intval($val % 256);
				if ($val<0) {
					// special opration for negative value. If the number id too big, PHP stores it into a signed integer. For example: crc32('coucou') => -256185401 instead of  4038781895. NegVal = BigVal - (MaxVal+1) = BigVal - 256^4
					$val = ($val - $z)/256 -1;
					$z = 256 + $z;
				} else {
					$val = ($val - $z)/256;
				}
			}
			$x .= chr($z);
		}
		$txt = substr_replace($txt, $x, $pos, $len);
	}
	
}

?>