<?php

function wysos_get_efnum_from_cid($cid)
{
//	drupal_set_message('wysos_get_efnum_from_cid('.$cid.')');

	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,"https://wyobiz.wy.gov/Business/FilingSearch.aspx");
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
//	curl_setopt($ch, CURLOPT_PROXY, $proxy);
	curl_setopt ($ch, CURLOPT_HEADER, 1);
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	$raw = curl_exec($ch);
	$pos = strpos($raw,'<!DOCTYPE html');
	$headers = trim(substr($raw,0,$pos-2));
	$body = trim(substr($raw,$pos));
	curl_close ($ch);
	unset($ch);	
	// Set Cookies
	preg_match_all('|Set-Cookie: (.*);|U', $headers, $results);    
	$cookies = implode(';', $results[1]);
//	echo 'cookies: '.print_r($cookies, true).'<br />';
	// Extract html

	$doc = new DOMDocument();
	libxml_use_internal_errors(true);
	$load = $doc->loadHTML($body);
	libxml_use_internal_errors(false);
	$inputs = $doc->getElementsByTagName('input');
    foreach($inputs as $input) {
		$data[$input->getAttribute('name')] = $input->getAttribute('value');
    } 
	$data['ctl00$MainContent$myScriptManager'] = 'ctl00$MainContent$UpdatePanel1|ctl00$MainContent$cmdSearch';
	$data['__LASTFOCUS'] = urlencode('');
	$data['__EVENTTARGET'] = '';
	$data['__EVENTARGUMENT'] = urlencode('');
	$data['ctl00$MainContent$txtFilingName'] = '';
	$data['ctl00$MainContent$searchOpt'] = 'chkSearchStartWith';
	$data['ctl00$MainContent$txtFilingID'] = $cid;
	$data['__ASYNCPOST'] = 'true';
	$data['ctl00$MainContent$cmdSearch'] = ' Search'; 
	unset($doc);
	
	$url = "https://wyobiz.wy.gov/Business/FilingSearch.aspx";
	$ch = curl_init();
	curl_setopt ($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_COOKIE,  $cookies);
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
	curl_setopt ($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch,CURLOPT_HTTPHEADER,array('X-Requested-With: XMLHttpRequest', 'X-MicrosoftAjax: Delta=true', 'Content-Type: application/x-www-form-urlencoded; charset=utf-8', 'Origin: https://wyobiz.wy.gov'));
//	curl_setopt($ch, CURLOPT_PROXY, $proxy);
	curl_setopt ($ch, CURLOPT_REFERER, 'https://wyobiz.wy.gov/Business/FilingSearch.aspx');
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_POST, true);
	curl_setopt ($ch, CURLOPT_FOLLOWLOCATION, true); // follow redirects 
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_setopt( $ch, CURLOPT_HTTPHEADER, array( 'Expect:' ) );
	
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data);		
	if($raw = curl_exec ($ch)) {
		$needle = 'FilingDetails.aspx?eFNum=';
		if(strpos($raw,$needle) > 0)
		{
			$start = strpos($raw,$needle) + strlen($needle);
			$end = strpos($raw,"'>",$start);
			$eFnum = substr($raw,$start,($end-$start));
			if(empty($eFnum)) {
				return FALSE;
			} else {
				db_query("UPDATE wra_companies SET eFNum='".$eFnum."' WHERE filing_num='".$cid."'");
				//echo "cid ".$cid."=".$eFnum;			
				// Set eFnum in mysql
				/*$num_upd = db_update('wra_companies')
					->fields(array('eFNum' => $eFnum))
					->condition('filing_num', $cid)
					->execute(); */
			}
		} else {
			//echo "No eFNum found for CID ".$cid;
			$eFNum = NULL;
		}
	}  else {
		drupal_set_message('Curl Error ('. curl_errno($ch) .'): ' .curl_error ( $ch ));
		return FALSE;
	}
	return $eFnum;
}
function wysos_get_ra_from_efnum($eFnum)
{
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,"https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum=".$eFnum);
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
	curl_setopt ($ch, CURLOPT_HEADER, 0);
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	$timenow = time();
	$raw = curl_exec($ch);
	$needle = '<span id="txtAgentName">';
	$start = strpos($raw,$needle) + strlen($needle);
	$end = strpos($raw,"</span>",$start);
	$ra_name = substr($raw,$start,($end-$start));

	$needle = '<span id="txtAgentAddress">';
	$start = strpos($raw,$needle) + strlen($needle);
	$end = strpos($raw,"</span>",$start);
	$ra_addr = substr($raw,$start,($end-$start));
	curl_close ($ch);
	unset($ch);	

	// If ra_name empty, skip and watchdog
	if(empty($ra_name)) {
		watchdog('wysos','Cannot get ra_name for eFNum='.$eFnum);
		return FALSE;				
	} else {
		// Find ra in ra_registered_agent
	//	$query = "SELECT * FROM wra_registered_agents WHERE name='".$ra_name."'";
		$result = db_query("SELECT * FROM wra_registered_agents WHERE name='".mysql_real_escape_string($ra_name)."'")->fetchObject();	
		if($result) {
			$ra_id = $result->ra_id;
		} else {
			// Is the ra_name a company?
			$result = db_query("SELECT * FROM wra_companies WHERE company_name='".mysql_real_escape_string($ra_name)."'")->fetchObject();	
			if($result) {
				// Insert ra as a company
				$result2 = db_query("INSERT wra_registered_agents (name, company_id) VALUES('".mysql_real_escape_string($ra_name)."', ".$result['company_id'].")");
				//$ra_id = db_last_insert_id('wra_registered_agents','ra_id');	
			} else {
				// Insert ra as an individual
				$result2 = db_query("INSERT wra_registered_agents (name, company_id) VALUES('".mysql_real_escape_string($ra_name)."', NULL)");
				//$ra_id = db_last_insert_id('wra_registered_agents','ra_id');	
			}
		}
		$result = db_query("SELECT c.ra_id, c.company_id, ra.name AS ra_name
			FROM wra_companies c
			LEFT JOIN wra_registered_agents ra ON c.ra_id=ra.ra_id
			WHERE eFNum=". $eFnum)->fetchObject();
		if($result->ra_id == $ra_id) {
			$result = db_query("UPDATE wra_companies SET last_ra_update=".$timenow." WHERE company_id=".$result->company_id);
		} else {
			// Change wra_companies ra_id
			$result = db_query("UPDATE wra_companies SET ra_id=".$ra_id.", last_ra_update=".$timenow." WHERE company_id=".$result->company_id);	
			// Add wwra_companies_log
			$result = db_query("INSERT wra_company_log SET company_id=".$rowCompany['company_id'].", log_type='RA Change', uid=2, old_value='".mysql_escape_string($rowCompany['ra_name']).
				"', new_value='".mysql_escape_string($ra_name)."', created=UNIX_TIMESTAMP()");
		}
		return $ra_id;
	}
}

function wysos_download_sos_file()
{
	$doc = new DOMDocument();
	$load = @$doc->loadHTMLFile("https://wyobiz.wy.gov/Business/Database.aspx");
	
	$inputs = $doc->getElementsByTagName('input');
    foreach($inputs as $input) {
		$data[$input->getAttribute('name')] = $input->getAttribute('value');
    }
	$data['ctl00$contentMain$DownloadButton'] = 'Download';
	unset($doc);
	
	$url = "https://wyobiz.wy.gov/Business/Database.aspx";
	$ch = curl_init();
	curl_setopt ($ch, CURLOPT_URL, $url);
	curl_setopt ($ch, CURLOPT_USERAGENT, "Mozilla/5.001 (windows; U; NT4.0; en-us) Gecko/25250101");
	curl_setopt ($ch, CURLOPT_HEADER, 0);
	curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_POST, true);
	curl_setopt ($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data);		
	$upload_dir = "/home/wyregnet/wysos/";
	$unzip_dir = "/tmp/";
	$filename = sprintf("Database%s.zip", date('Y-m-d'));
	if($raw = curl_exec ($ch)) {
	//		echo "Putting...\r\n";
//		flush();
//		echo 'Contents: ' . substr($raw, 1, 200);
//		flush();
		if(!file_put_contents($upload_dir.$filename, $raw)) {
//			echo "Error\r\n";
			return;
		}
	drupal_set_message('Downloaded ' .$unzip_dir.$filename);
//	flush();
	}
	
	// Open ZIP
	$zip = zip_open($upload_dir.$filename);
	if ($zip) {
	  while ($zip_entry = zip_read($zip)) {
		$fp = fopen($unzip_dir . zip_entry_name($zip_entry), "w");
//		echo 'Creating ' . zip_entry_name($zip_entry) . "\n";
		if (zip_entry_open($zip, $zip_entry, "r")) {
		  while($buf = zip_entry_read($zip_entry, 1024))
		  {
			  fwrite($fp,$buf);
		  }
		  
//		  $buf = zip_entry_read($zip_entry, zip_entry_filesize($zip_entry));
//		  fwrite($fp,$buf);
		  zip_entry_close($zip_entry);
		  fclose($fp);
		}
	  }
	}
	zip_close($zip);
	

	// Import
//	echo "Importing...\r\n";
//	flush();
	
	// WySosFILING
	$query = "DROP TABLE IF EXISTS WySosFILING;";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query1: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "CREATE TABLE WySosFILING (
	FILING_ID INTEGER(11), 
	FILING_TYPE VARCHAR(50), 
	FILING_SUBTYPE VARCHAR(50),
	WORD_DESIGN_TYPE VARCHAR(50), 
	DURATION_TERM_TYPE VARCHAR(50), 
	STATUS VARCHAR(50), 
	SUB_STATUS VARCHAR(50), 
	STANDING_TAX VARCHAR(50), 
	STANDING_RA VARCHAR(50), 
	STANDING_OTHER VARCHAR(50), 
	PURPOSE VARCHAR(255), 
	APPLICANT_TYPE VARCHAR(50), 
	FILING_NUM CHAR(14), 
	FILING_NAME VARCHAR(256), 
	OLD_NAME VARCHAR(256), 
	FICTITIOUS_NAME VARCHAR(256), 
	DOMESTIC_YN CHAR(1), 
	tFILING_DATE VARCHAR(20), 
	tDELAYED_EFFECTIVE_DATE VARCHAR(20), 
	tEXPIRATION_DATE VARCHAR(20), 
	tINACTIVE_DATE VARCHAR(20), 
	tRA_RESIGN_CERT_LETTER_DATE VARCHAR(20), 
	CONVERTED_YN CHAR(1), 
	CONVERTED_FROM VARCHAR(128), 
	CONVERTED_FROM_NAME VARCHAR(256), 
	tCONVERTED_DATE VARCHAR(20), 
	ISSUE_ON_RECORD_YN CHAR(1), 
	TRANSFERRED_TO VARCHAR(128), 
	tTRANSFERRED_DATE VARCHAR(20), 
	FORMATION_LOCALE VARCHAR(128), 
	CONTINUED_FROM_LOCALE VARCHAR(128), 
	DOMESTICATED_FROM_LOCALE VARCHAR(50), 
	tFORM_HOME_JURIS_DATE VARCHAR(20), 
	COMMON_SHARES VARCHAR(20), 
	COMMON_PAR_VALUE DECIMAL(8,2),
	PREFERRED_SHARES VARCHAR(20),
	PREFERRED_PAR_VALUE DECIMAL(8,2), 
	ADDITIONAL_STOCK_YN CHAR(1),
	PRINCIPLE_ADDR1 VARCHAR(128),
	PRINCIPLE_ADDR2 VARCHAR(128),
	PRINCIPLE_ADDR3 VARCHAR(128),
	PRINCIPLE_CITY VARCHAR(80),
	PRINCIPLE_STATE VARCHAR(50),
	PRINCIPLE_POSTAL_CODE VARCHAR(50),
	PRINCIPLE_COUNTRY VARCHAR(128),
	MAIL_ADDR1 VARCHAR(128),
	MAIL_ADDR2 VARCHAR(128),
	MAIL_ADDR3 VARCHAR(128),
	MAIL_CITY VARCHAR(80),
	MAIL_STATE VARCHAR(50),
	MAIL_POSTAL_CODE VARCHAR(50),
	MAIL_COUNTRY VARCHAR(128),
	STATE_OF_ORG VARCHAR(50),
	tORG_DATE VARCHAR(20),
	REG_US_OFFICE_YN CHAR(1),
	tREG_US_DATE VARCHAR(20),
	REG_US_SERIAL_NUM VARCHAR(30),
	REG_US_STATUS VARCHAR(30),
	REG_US_APP_REFUSED_YN CHAR(1),
	tFIRST_USED_ANYWHERE_DATE VARCHAR(20),
	tFIRST_USED_WYO_DATE VARCHAR(20),
	AR_EXEMPT_YN CHAR(1),
	TRADEMARK_KEYWORDS VARCHAR(1000),
	PRIMARY KEY (FILING_ID), 
	UNIQUE (FILING_NUM),
	INDEX (FILING_NAME));";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query2: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$options = array(
    'target' => 'default',
    'fetch' => PDO::FETCH_OBJ,
//    'return' => Database::RETURN_NULL,
    'return' => NULL,
    'throw_exception' => TRUE
  );
  	$query = "LOAD DATA INFILE '".$unzip_dir."FILING.csv' IGNORE INTO TABLE WySosFILING FIELDS TERMINATED BY '|' IGNORE 1 LINES;";
	$result = db_query($query, array(), $options);
	
	if (!$result) {
		$message  = 'Invalid query3: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "ALTER TABLE WySosFILING 
	ADD COLUMN FILING_DATE DATE AFTER tFILING_DATE,
	ADD COLUMN DELAYED_EFFECTIVE_DATE DATE AFTER tDELAYED_EFFECTIVE_DATE,
	ADD COLUMN EXPIRATION_DATE DATE AFTER tEXPIRATION_DATE,
	ADD COLUMN INACTIVE_DATE DATE AFTER tINACTIVE_DATE,
	ADD COLUMN RA_RESIGN_CERT_LETTER_DATE DATE AFTER tRA_RESIGN_CERT_LETTER_DATE, 
	ADD COLUMN CONVERTED_DATE DATE AFTER tCONVERTED_DATE, 
	ADD COLUMN TRANSFERRED_DATE DATE AFTER tTRANSFERRED_DATE, 
	ADD COLUMN FORM_HOME_JURIS_DATE DATE AFTER tFORM_HOME_JURIS_DATE, 
	ADD COLUMN ORG_DATE DATE AFTER tORG_DATE,
	ADD COLUMN REG_US_DATE DATE AFTER tREG_US_DATE,
	ADD COLUMN FIRST_USED_ANYWHERE_DATE DATE AFTER tFIRST_USED_ANYWHERE_DATE,
	ADD COLUMN FIRST_USED_WYO_DATE DATE AFTER tFIRST_USED_WYO_DATE ;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query4: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "UPDATE IGNORE WySosFILING 
	SET FILING_DATE = IF(tFILING_DATE,CONCAT(SUBSTRING(tFILING_DATE, 7, 4),'-',SUBSTRING(tFILING_DATE, 1, 2),'-',SUBSTRING(tFILING_DATE, 4, 2)),NULL),
	DELAYED_EFFECTIVE_DATE = IF(tDELAYED_EFFECTIVE_DATE,CONCAT(SUBSTRING(tDELAYED_EFFECTIVE_DATE , 7, 4),'-',SUBSTRING(tDELAYED_EFFECTIVE_DATE , 1, 2),'-',SUBSTRING(tDELAYED_EFFECTIVE_DATE , 4, 2)),NULL),
	EXPIRATION_DATE = IF(tEXPIRATION_DATE,CONCAT(SUBSTRING(tEXPIRATION_DATE , 7, 4),'-',SUBSTRING(tEXPIRATION_DATE , 1, 2),'-',SUBSTRING(tEXPIRATION_DATE , 4, 2)),NULL),
	INACTIVE_DATE = IF(tINACTIVE_DATE,CONCAT(SUBSTRING(tINACTIVE_DATE , 7, 4),'-',SUBSTRING(tINACTIVE_DATE, 1, 2),'-',SUBSTRING(tINACTIVE_DATE, 4, 2)),NULL),
	RA_RESIGN_CERT_LETTER_DATE = IF(tRA_RESIGN_CERT_LETTER_DATE,CONCAT(SUBSTRING(tRA_RESIGN_CERT_LETTER_DATE , 7, 4),'-',SUBSTRING(tRA_RESIGN_CERT_LETTER_DATE , 1, 2),'-',SUBSTRING(tRA_RESIGN_CERT_LETTER_DATE , 4, 2)),NULL),
	CONVERTED_DATE = IF(tCONVERTED_DATE,CONCAT(SUBSTRING(tCONVERTED_DATE, 7, 4),'-',SUBSTRING(tCONVERTED_DATE, 1, 2),'-',SUBSTRING(tCONVERTED_DATE, 4, 2)),NULL),
	TRANSFERRED_DATE = IF(tTRANSFERRED_DATE,CONCAT(SUBSTRING(tTRANSFERRED_DATE , 7, 4),'-',SUBSTRING(tTRANSFERRED_DATE , 1, 2),'-',SUBSTRING(tTRANSFERRED_DATE , 4, 2)),NULL),
	FORM_HOME_JURIS_DATE = IF(tFORM_HOME_JURIS_DATE,CONCAT(SUBSTRING(tFORM_HOME_JURIS_DATE , 7, 4),'-',SUBSTRING(tFORM_HOME_JURIS_DATE , 1, 2),'-',SUBSTRING(tFORM_HOME_JURIS_DATE , 4, 2)),NULL),
	ORG_DATE = IF(tORG_DATE,CONCAT(SUBSTRING(tORG_DATE, 7, 4),'-',SUBSTRING(tORG_DATE , 1, 2),'-',SUBSTRING(tORG_DATE, 4, 2)),NULL),
	REG_US_DATE = IF(tREG_US_DATE,CONCAT(SUBSTRING(tREG_US_DATE, 7, 4),'-',SUBSTRING(tREG_US_DATE , 1, 2),'-',SUBSTRING(tREG_US_DATE, 4, 2)),NULL),
	FIRST_USED_ANYWHERE_DATE = IF(tFIRST_USED_ANYWHERE_DATE,CONCAT(SUBSTRING(tFIRST_USED_ANYWHERE_DATE, 7, 4),'-',SUBSTRING(tFIRST_USED_ANYWHERE_DATE, 1, 2),'-',SUBSTRING(tFIRST_USED_ANYWHERE_DATE, 4, 2)),NULL),
	FIRST_USED_WYO_DATE = IF(tFIRST_USED_WYO_DATE,CONCAT(SUBSTRING(tFIRST_USED_WYO_DATE, 7, 4),'-',SUBSTRING(tFIRST_USED_WYO_DATE, 1, 2),'-',SUBSTRING(tFIRST_USED_WYO_DATE, 4, 2)),NULL);";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query5: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "ALTER TABLE WySosFILING 
	DROP COLUMN tFILING_DATE,
	DROP COLUMN tDELAYED_EFFECTIVE_DATE,
	DROP COLUMN tEXPIRATION_DATE,
	DROP COLUMN tINACTIVE_DATE,
	DROP COLUMN tRA_RESIGN_CERT_LETTER_DATE, 
	DROP COLUMN tCONVERTED_DATE, 
	DROP COLUMN tTRANSFERRED_DATE, 
	DROP COLUMN tFORM_HOME_JURIS_DATE, 
	DROP COLUMN tORG_DATE,
	DROP COLUMN tREG_US_DATE,
	DROP COLUMN tFIRST_USED_ANYWHERE_DATE,
	DROP COLUMN tFIRST_USED_WYO_DATE ;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query6: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	// WySosPARTY
	$query = "DROP TABLE IF EXISTS WySosPARTY;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query7: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "CREATE TABLE WySosPARTY (
	PARTY_ID INT, 
	PARTY_TYPE VARCHAR(50), 
	SOURCE_ID INT, 
	SOURCE_TYPE VARCHAR(50), 
	ORG_NAME VARCHAR(256), 
	FIRST_NAME VARCHAR(50), 
	MIDDLE_NAME VARCHAR(30), 
	LAST_NAME VARCHAR(100), 
	INDIVIDUAL_TITLE VARCHAR(100), 
	ADDR1 VARCHAR(500), 
	ADDR2 VARCHAR(100), 
	ADDR3 VARCHAR(100),
	CITY VARCHAR(60), 
	COUNTY VARCHAR(60), 
	STATE VARCHAR(50), 
	POSTAL_CODE VARCHAR(30),
	COUNTRY VARCHAR(100), 
	PRIMARY KEY (PARTY_ID), 
	INDEX (SOURCE_ID));";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query8: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "LOAD DATA INFILE '".$unzip_dir."PARTY.csv' IGNORE INTO TABLE WySosPARTY FIELDS TERMINATED BY '|' IGNORE 1 LINES;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query9: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	// WySosFILING_ANNUAL_REPORT
	$query = "DROP TABLE IF EXISTS WySosFILING_ANNUAL_REPORT;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query10: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "CREATE TABLE WySosFILING_ANNUAL_REPORT (
	FILING_ANNUAL_REPORT_ID INT, 
	FILING_ID INT, 
	STATUS VARCHAR(50), 
	ANNUAL_REPORT_NUM VARCHAR(20), 
	FILING_YEAR INT, 
	tFILING_DATE VARCHAR(20), 
	LICENSE_TAX_AMOUNT DECIMAL(8,2), 
	PRIMARY KEY (FILING_ANNUAL_REPORT_ID), 
	INDEX (FILING_ID));";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query11: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "LOAD DATA INFILE '".$unzip_dir."FILING_ANNUAL_REPORT.csv' IGNORE INTO TABLE WySosFILING_ANNUAL_REPORT FIELDS TERMINATED BY '|' IGNORE 1 LINES;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query12: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "ALTER TABLE WySosFILING_ANNUAL_REPORT
	ADD COLUMN FILING_DATE DATE AFTER tFILING_DATE;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query13: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "UPDATE IGNORE WySosFILING_ANNUAL_REPORT 
	SET FILING_DATE = IF(tFILING_DATE,CONCAT(SUBSTRING(tFILING_DATE, 7, 4),'-',SUBSTRING(tFILING_DATE, 1, 2),'-',SUBSTRING(tFILING_DATE, 4, 2)),NULL);";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query14: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "ALTER TABLE WySosFILING_ANNUAL_REPORT
	DROP COLUMN tFILING_DATE;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query15: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Change Other types to filings
	$query = "INSERT INTO wra_company_log (company_id, log_type, uid, old_value, new_value, created) 
	SELECT c.company_id, 'Other FILING_TYPE Change', 2, 'Other', f.FILING_TYPE, UNIX_TIMESTAMP() 
	from wra_companies c 
	INNER JOIN WySosFILING f ON c.company_name=f.filing_name
	WHERE c.filing_type = 'Other'";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query16: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies c
	INNER JOIN WySosFILING f ON c.company_name=f.filing_name
	SET c.filing_num=f.filing_num , c.formation_locale=f.formation_locale, c.filing_id=f.filing_id, c.filing_date=f.filing_date, c.inactive_date=f.inactive_date, c.filing_type=f.filing_type,
	c.sos_status=CASE WHEN (f.STANDING_TAX = 'Delinquent' AND f.STATUS = 'Active') THEN 'Delinquent - Tax'
	  WHEN (f.STANDING_RA = 'Delinquent' AND f.STATUS = 'Active') THEN 'Delinquent - Registered Agent'
	  WHEN (f.STANDING_OTHER = 'Delinquent' AND f.STATUS = 'Active') THEN 'Delinquent - Other'
	  ELSE f.STATUS END, c.principal_addr1=f.principle_addr1, c.principal_addr2=f.principle_addr2, c.principal_addr3=f.principle_addr3, c.principal_city=f.principle_city,
	c.principal_state=f.principle_state, c.principal_postal_code=f.principle_postal_code, c.principal_country=f.principle_country, 
	c.mail_addr1=f.mail_addr1, c.mail_addr2=f.mail_addr2, c.mail_addr3=f.mail_addr3, c.mail_city=f.mail_city, c.mail_state=f.mail_state, c.mail_postal_code=f.mail_postal_code, c.mail_country=f.mail_country, c.updated=UNIX_TIMESTAMP()
	WHERE c.filing_type = 'Other'";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query17: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	// wra_companies
	$query = "INSERT INTO wra_companies (company_name, filing_num, formation_locale, filing_id, filing_date, inactive_date, filing_type, sos_status, principal_addr1, principal_addr2, principal_addr3, principal_city,
	principal_state, principal_postal_code, principal_country, mail_addr1, mail_addr2, mail_addr3, mail_city, mail_state, mail_postal_code, mail_country, created)
	SELECT FILING_NAME, FILING_NUM, FORMATION_LOCALE, FILING_ID, FILING_DATE, INACTIVE_DATE, FILING_TYPE, 
	CASE WHEN (WySosFILING.STANDING_TAX = 'Delinquent' AND WySosFILING.STATUS = 'Active') THEN 'Delinquent - Tax'
	  WHEN (WySosFILING.STANDING_RA = 'Delinquent' AND WySosFILING.STATUS = 'Active') THEN 'Delinquent - Registered Agent'
	  WHEN (WySosFILING.STANDING_OTHER = 'Delinquent' AND WySosFILING.STATUS = 'Active') THEN 'Delinquent - Other'
	  ELSE WySosFILING.STATUS END AS sos_status, PRINCIPLE_ADDR1, PRINCIPLE_ADDR2, PRINCIPLE_ADDR3, PRINCIPLE_CITY, PRINCIPLE_STATE, PRINCIPLE_POSTAL_CODE, PRINCIPLE_COUNTRY,
	  MAIL_ADDR1, MAIL_ADDR2, MAIL_ADDR3, MAIL_CITY, MAIL_STATE, MAIL_POSTAL_CODE, MAIL_COUNTRY, UNIX_TIMESTAMP()
	 FROM WySosFILING WHERE FILING_ID NOT IN(SELECT filing_id FROM wra_companies WHERE filing_id IS NOT NULL)";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query16: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update FILING_ID
	$query = "INSERT INTO wra_company_log (company_id, log_type, uid, old_value, new_value, created) 
	SELECT c.company_id, 'FILING_ID Change', 2, c.filing_id, WySosFILING.FILING_ID, UNIX_TIMESTAMP() from wra_companies c 
	INNER JOIN WySosFILING ON c.filing_id = WySosFILING.FILING_ID
	WHERE c.FILING_NUM IS NOT NULL AND c.filing_id!=WySosFILING.FILING_ID";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query17: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_NUM = wra_companies.filing_num
	SET wra_companies.filing_id=WySosFILING.FILING_ID, updated = UNIX_TIMESTAMP()
	WHERE wra_companies.FILING_NUM IS NOT NULL AND wra_companies.filing_id!=WySosFILING.FILING_ID";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query18: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update Filing_Name
	$query = "INSERT INTO wra_company_log (company_id, log_type, uid, old_value, new_value, created) 
	SELECT wra_companies.company_id, 'Name Change', 2, wra_companies.company_name, WySosFILING.FILING_NAME, UNIX_TIMESTAMP() from wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	WHERE wra_companies.FILING_NUM IS NOT NULL AND wra_companies.company_name!=WySosFILING.FILING_NAME";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query19: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_ID = wra_companies.filing_id
	SET wra_companies.company_name=WySosFILING.FILING_NAME, updated = UNIX_TIMESTAMP()
	WHERE wra_companies.FILING_NUM IS NOT NULL AND wra_companies.company_name!=WySosFILING.FILING_NAME";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query20: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update FORMATION_LOCALE
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_ID = wra_companies.filing_id
	SET wra_companies.formation_locale=WySosFILING.FORMATION_LOCALE, updated = UNIX_TIMESTAMP()
	WHERE wra_companies.filing_id IS NOT NULL AND (wra_companies.formation_locale!=WySosFILING.FORMATION_LOCALE OR wra_companies.formation_locale IS NULL)";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query21: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update FILING_DATE
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_ID = wra_companies.filing_id
	SET wra_companies.filing_date=WySosFILING.FILING_DATE, updated = UNIX_TIMESTAMP()
	WHERE wra_companies.filing_id IS NOT NULL AND wra_companies.filing_date!=WySosFILING.FILING_DATE";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query22: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update INACTIVE_DATE
	$query = "INSERT INTO wra_company_log (company_id, log_type, uid, old_value, new_value, created) 
	SELECT wra_companies.company_id, 'Inactive Date Change', 2, wra_companies.inactive_date, WySosFILING.INACTIVE_DATE, UNIX_TIMESTAMP() from wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	WHERE wra_companies.FILING_NUM IS NOT NULL AND (wra_companies.inactive_date!=WySosFILING.INACTIVE_DATE OR (wra_companies.inactive_date IS NULL AND WySosFILING.INACTIVE_DATE IS NOT NULL) OR (wra_companies.inactive_date IS NOT NULL AND WySosFILING.INACTIVE_DATE IS NULL))";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query23: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_ID = wra_companies.filing_id
	SET wra_companies.inactive_date=WySosFILING.INACTIVE_DATE, updated = UNIX_TIMESTAMP()
	WHERE wra_companies.FILING_NUM IS NOT NULL AND (wra_companies.inactive_date!=WySosFILING.INACTIVE_DATE OR (wra_companies.inactive_date IS NULL AND WySosFILING.INACTIVE_DATE IS NOT NULL) OR (wra_companies.inactive_date IS NOT NULL AND WySosFILING.INACTIVE_DATE IS NULL))";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query24: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update last_ar
	$query = "UPDATE wra_companies LEFT JOIN WySosFILING ON WySosFILING.FILING_ID = wra_companies.filing_id 
	SET wra_companies.last_ar_year =  (SELECT Max(WySosFILING_ANNUAL_REPORT.FILING_YEAR) FROM WySosFILING_ANNUAL_REPORT WHERE WySosFILING_ANNUAL_REPORT.FILING_ID = wra_companies.filing_id),
		wra_companies.next_ar_due =  UNIX_TIMESTAMP(IF(wra_companies.last_ar_year IS NULL,
		((PERIOD_ADD(EXTRACT(YEAR_MONTH FROM (ADDDATE(WySosFILING.FILING_DATE, INTERVAL 1 YEAR))),0)*100)+1),
		STR_TO_DATE(CONCAT(wra_companies.last_ar_year+1,'-',MONTH(WySosFILING.FILING_DATE),'-01'),'%%Y-%%m-%%d')))";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query25: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update FILING_TYPE
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_NUM = wra_companies.filing_num
	SET wra_companies.filing_type=WySosFILING.FILING_TYPE, updated = UNIX_TIMESTAMP()
	WHERE wra_companies.FILING_NUM IS NOT NULL AND wra_companies.filing_type!=WySosFILING.FILING_TYPE";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query26: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update Sos_status
	$query = "UPDATE wra_companies
	JOIN WySosFILING ON WySosFILING.FILING_NUM = wra_companies.filing_num
	SET wra_companies.sos_status=
	CASE WHEN (WySosFILING.STANDING_TAX = 'Delinquent' AND WySosFILING.STATUS = 'Active' AND wra_companies.sos_status != 'Delinquent - Tax') THEN 'Delinquent - Tax'
	  WHEN (WySosFILING.STANDING_RA = 'Delinquent' AND WySosFILING.STATUS = 'Active' AND wra_companies.sos_status != 'Delinquent - Registered Agent') THEN 'Delinquent - Registered Agent'
	  WHEN (WySosFILING.STANDING_OTHER = 'Delinquent' AND WySosFILING.STATUS = 'Active' AND wra_companies.sos_status != 'Delinquent - Other') THEN 'Delinquent - Other'
	  ELSE WySosFILING.STATUS END,
	 updated = UNIX_TIMESTAMP()
	WHERE wra_companies.FILING_NUM IS NOT NULL AND ((wra_companies.sos_status !=
	CASE WHEN (WySosFILING.STANDING_TAX = 'Delinquent' AND WySosFILING.STATUS = 'Active' ) THEN 'Delinquent - Tax'
	  WHEN (WySosFILING.STANDING_RA = 'Delinquent' AND WySosFILING.STATUS = 'Active' ) THEN 'Delinquent - Registered Agent'
	  WHEN (WySosFILING.STANDING_OTHER = 'Delinquent' AND WySosFILING.STATUS = 'Active' ) THEN 'Delinquent - Other'
	  ELSE WySosFILING.STATUS END) OR (wra_companies.sos_status IS NULL));";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query27: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	
	// Update Principal Office
	$query = "INSERT INTO wra_company_log (company_id, log_type, uid, old_value, new_value, created) 
	SELECT wra_companies.company_id, 'Principal Office Change', 2, CONCAT(wra_companies.principal_addr1,'|', wra_companies.principal_addr2, '|', wra_companies.principal_addr3, '|', wra_companies.principal_city, '|', wra_companies.principal_state, '|', wra_companies.principal_postal_code, '|', wra_companies.principal_country), 
	CONCAT(WySosFILING.PRINCIPLE_ADDR1,'|',WySosFILING.PRINCIPLE_ADDR2,'|',WySosFILING.PRINCIPLE_ADDR3,'|',WySosFILING.PRINCIPLE_CITY,'|',WySosFILING.PRINCIPLE_STATE,'|',WySosFILING.PRINCIPLE_POSTAL_CODE, '|', WySosFILING.PRINCIPLE_COUNTRY), UNIX_TIMESTAMP() from wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	WHERE wra_companies.principal_addr1 != WySosFILING.PRINCIPLE_ADDR1 OR wra_companies.principal_addr1 IS NULL
	OR  wra_companies.principal_addr2 != WySosFILING.PRINCIPLE_ADDR2 OR  wra_companies.principal_addr2 IS NULL
	OR wra_companies.principal_addr3 != WySosFILING.PRINCIPLE_ADDR3 OR wra_companies.principal_addr3 IS NULL
	OR wra_companies.principal_city != WySosFILING.PRINCIPLE_CITY OR wra_companies.principal_city IS NULL
	OR wra_companies.principal_state != WySosFILING.PRINCIPLE_STATE OR wra_companies.principal_state IS NULL
	OR wra_companies.principal_postal_code != WySosFILING.PRINCIPLE_POSTAL_CODE OR wra_companies.principal_postal_code IS NULL
	OR wra_companies.principal_country != WySosFILING.PRINCIPLE_COUNTRY OR (wra_companies.principal_country IS NULL AND WySosFILING.PRINCIPLE_COUNTRY IS NOT NULL);";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query28: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET wra_companies.principal_addr1 = WySosFILING.PRINCIPLE_ADDR1, wra_companies.updated = UNIX_TIMESTAMP()
	WHERE wra_companies.principal_addr1 != WySosFILING.PRINCIPLE_ADDR1 OR wra_companies.principal_addr1 IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query29: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.principal_addr2 = WySosFILING.PRINCIPLE_ADDR2, wra_companies.updated = UNIX_TIMESTAMP()
	WHERE wra_companies.principal_addr2 != WySosFILING.PRINCIPLE_ADDR2 OR wra_companies.principal_addr2 IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query30: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.principal_addr3 = WySosFILING.PRINCIPLE_ADDR3, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.principal_addr3 != WySosFILING.PRINCIPLE_ADDR3 OR wra_companies.principal_addr3 IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query31: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.principal_city = WySosFILING.PRINCIPLE_CITY, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.principal_city != WySosFILING.PRINCIPLE_CITY OR wra_companies.principal_city IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query32: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.principal_state = WySosFILING.PRINCIPLE_STATE, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.principal_state != WySosFILING.PRINCIPLE_STATE OR wra_companies.principal_state IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query33: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.principal_postal_code = WySosFILING.PRINCIPLE_POSTAL_CODE, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.principal_postal_code != WySosFILING.PRINCIPLE_POSTAL_CODE OR wra_companies.principal_postal_code IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query34: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.principal_country = WySosFILING.PRINCIPLE_COUNTRY, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.principal_country != WySosFILING.PRINCIPLE_COUNTRY OR wra_companies.principal_country IS NULL"; 	
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query35: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET wra_companies.principal_addr1 = WySosFILING.PRINCIPLE_ADDR1, wra_companies.principal_addr2 = WySosFILING.PRINCIPLE_ADDR2, 
		wra_companies.principal_addr3 = WySosFILING.PRINCIPLE_ADDR3, wra_companies.principal_city = WySosFILING.PRINCIPLE_CITY, 
		wra_companies.principal_state = WySosFILING.PRINCIPLE_STATE, wra_companies.principal_postal_code = WySosFILING.PRINCIPLE_POSTAL_CODE, 
		wra_companies.principal_country = WySosFILING.PRINCIPLE_COUNTRY, wra_companies.updated = UNIX_TIMESTAMP()
	WHERE wra_companies.principal_addr1 != WySosFILING.PRINCIPLE_ADDR1 OR wra_companies.principal_addr1 IS NULL
	OR  wra_companies.principal_addr2 != WySosFILING.PRINCIPLE_ADDR2 OR  wra_companies.principal_addr2 IS NULL
	OR wra_companies.principal_addr3 != WySosFILING.PRINCIPLE_ADDR3 OR wra_companies.principal_addr3 IS NULL
	OR wra_companies.principal_city != WySosFILING.PRINCIPLE_CITY OR wra_companies.principal_city IS NULL
	OR wra_companies.principal_state != WySosFILING.PRINCIPLE_STATE OR wra_companies.principal_state IS NULL
	OR wra_companies.principal_postal_code != WySosFILING.PRINCIPLE_POSTAL_CODE OR wra_companies.principal_postal_code IS NULL
	OR wra_companies.principal_country != WySosFILING.PRINCIPLE_COUNTRY OR (wra_companies.principal_country IS NULL AND WySosFILING.PRINCIPLE_COUNTRY IS NOT NULL);";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query36: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update Mailing Office
	$query = "INSERT INTO wra_company_log (company_id, log_type, uid, old_value, new_value, created) 
	SELECT wra_companies.company_id, 'Mail Office Change', 2, CONCAT(wra_companies.mail_addr1,'|', wra_companies.mail_addr2, '|', wra_companies.mail_addr3, '|', wra_companies.mail_city, '|', wra_companies.mail_state, '|', wra_companies.mail_postal_code, '|', wra_companies.mail_country), 
	
	CONCAT(WySosFILING.MAIL_ADDR1, '|', WySosFILING.MAIL_ADDR2, '|', WySosFILING.MAIL_ADDR3, '|', WySosFILING.MAIL_CITY,'|',WySosFILING.MAIL_STATE, '|', WySosFILING.MAIL_POSTAL_CODE, '|', WySosFILING.MAIL_COUNTRY), UNIX_TIMESTAMP() from wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	WHERE wra_companies.mail_addr1 != WySosFILING.MAIL_ADDR1 OR wra_companies.mail_addr1 IS NULL
	OR  wra_companies.mail_addr2 != WySosFILING.MAIL_ADDR2 OR  wra_companies.mail_addr2 IS NULL
	OR wra_companies.mail_addr3 != WySosFILING.MAIL_ADDR3 OR wra_companies.mail_addr3 IS NULL
	OR wra_companies.mail_city != WySosFILING.MAIL_CITY OR wra_companies.mail_city IS NULL
	OR wra_companies.mail_state != WySosFILING.MAIL_STATE OR wra_companies.mail_state IS NULL
	OR wra_companies.mail_postal_code != WySosFILING.MAIL_POSTAL_CODE OR wra_companies.mail_postal_code IS NULL
	OR wra_companies.mail_country != WySosFILING.MAIL_COUNTRY OR (wra_companies.mail_country IS NULL AND WySosFILING.MAIL_COUNTRY IS NOT NULL);";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query37: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET wra_companies.mail_addr1 = WySosFILING.MAIL_ADDR1, wra_companies.updated = UNIX_TIMESTAMP()
	WHERE wra_companies.mail_addr1 != WySosFILING.MAIL_ADDR1 OR wra_companies.mail_addr1 IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query38: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.mail_addr2 = WySosFILING.MAIL_ADDR2, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.mail_addr2 != WySosFILING.MAIL_ADDR2 OR wra_companies.mail_addr2 IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query39: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.mail_addr3 = WySosFILING.MAIL_ADDR3, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.mail_addr3 != WySosFILING.MAIL_ADDR3 OR wra_companies.mail_addr3 IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query40: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.mail_city = WySosFILING.MAIL_CITY, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.mail_city != WySosFILING.MAIL_CITY OR wra_companies.mail_city IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query41: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.mail_state = WySosFILING.MAIL_STATE, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.mail_state != WySosFILING.MAIL_STATE OR wra_companies.mail_state IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query42: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.mail_postal_code = WySosFILING.MAIL_POSTAL_CODE, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.mail_postal_code != WySosFILING.MAIL_POSTAL_CODE OR wra_companies.mail_postal_code IS NULL";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query43: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "UPDATE wra_companies 
	INNER JOIN WySosFILING ON wra_companies.filing_id = WySosFILING.FILING_ID
	SET  wra_companies.mail_country = WySosFILING.MAIL_COUNTRY, wra_companies.updated = UNIX_TIMESTAMP() 
	WHERE wra_companies.mail_country != WySosFILING.MAIL_COUNTRY OR wra_companies.mail_country IS NULL";
	$result = db_query($query);

	if (!$result) {
		$message  = 'Invalid query44: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	// Update Renewals
	$query = "DELETE FROM wra_renewals WHERE due_date < DATE_SUB(CURDATE(),INTERVAL 24 MONTH) AND payment_date IS NULL;";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query45: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		die($message);
	}
	
	db_query("UPDATE wra_companies SET nominee_sku=NULL, nominee_price=NULL WHERE nominee_sku=''");
	db_query("UPDATE wra_companies SET nominee_sku=NULL, nominee_price=NULL WHERE nominee_price=0");
	
	$query = "UPDATE wra_renewals r	
	INNER JOIN users u ON u.uid = r.uid
	INNER JOIN wra_companies c ON r.FILING_ID=c.filing_id
	LEFT JOIN wra_suites s ON s.company_id = c.company_id
	SET r.filing_name=c.company_name,
	r.last_ar=c.last_ar_year,
	r.next_ar_due=FROM_UNIXTIME(c.next_ar_due),
	r.filing_status=c.sos_status,
	r.ar_price=If(c.last_ar_year<r.ar_year Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation',25.00,If(c.filing_type='Statutory Trust',100.00,50.00)),Null),
	r.nominee_product=c.nominee_sku,
	r.nominee_price=c.nominee_price,
	r.ra_price=c.ra_price,
	r.mail_suite=s.suite_no,
	r.billing_period=s.billing_period,
	r.vo_price=s.price,
	r.Detail1=If(c.last_ar_year<r.ar_year Or IsNull(c.last_ar_year),'Annual Report',NULL),
	r.Detail1Price=If(c.last_ar_year<r.ar_year Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation','$25.00',If(c.filing_type='Statutory Trust','$100.00','$50.00')),Null),
	r.Detail2=If(IsNull(c.ra_price),Null,'Registered Agent'),
	r.Detail2Price=CONCAT('$',Format(c.ra_price,2)),
	r.Detail3=If(IsNull(c.nominee_price),Null,'Nominee Privacy'),
	r.Detail3Price=CONCAT('$',FORMAT(c.nominee_price,2)),
	r.Detail4=If(Not IsNull(s.price) And s.billing_period='year','Virtual Office',Null),
	r.Detail4Price=CONCAT('$',If(Not IsNull(s.price) And s.billing_period='year',s.price,Null)),
	r.Detail6=If(c.sos_status='Inactive - Administratively Dissolved (Tax)','Reinstatement',NULL),
	r.Detail6Price=If(c.sos_status='Inactive - Administratively Dissolved (Tax)','$50.00',NULL),
	r.reinstate_price=If(c.sos_status='Inactive - Administratively Dissolved (Tax)',50.00,NULL),
	r.last_modified=NOW(),
	
	r.total_due=If(IsNull(c.nominee_price),0,c.nominee_price)+If(IsNull(c.ra_price),0,c.ra_price)+If(s.billing_period='year' And Not IsNull(s.price),s.price,0)+If(c.last_ar_year<r.ar_year Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation',25,If(c.filing_type='Statutory Trust',100.00,50.00))+If(c.sos_status='Inactive - Administratively Dissolved (Tax)',50,0),0),
	
	r.ttotal_due=CONCAT('$',Format(If(IsNull(c.nominee_price),0,c.nominee_price)+If(IsNull(c.ra_price),0,c.ra_price)+If(s.billing_period='year' And Not IsNull(s.price),s.price,0)+If(c.last_ar_year<r.ar_year Or IsNull(c.last_ar_year),If(c.filing_type='NonProfit Corporation',25,If(c.filing_type='Statutory Trust',100.00,50.00))+If(c.sos_status='Inactive - Administratively Dissolved (Tax)',50,0),0),2))
	WHERE r.payment_date IS NULL;";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query46: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		die($message);
	}
	
	
	// Update Registered Agent Names
	$query = "UPDATE wra_registered_agents r SET r.name=(SELECT company_name FROM wra_companies c WHERE c.company_id=r.company_id) WHERE company_id IS NOT NULL";
	$result = db_query($query);
	
	if (!$result) {
		$message  = 'Invalid query47: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		die($message);
	}

	
	// Generate Tables
	$query = 'CALL generateSearchTables();';
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query48: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}
	
	// Update User Balances
	$query = "DELETE FROM profile_values WHERE fid=28";
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query49: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}

	$query = "INSERT profile_values (fid,uid,value) (SELECT 28, u.uid, (SELECT IFNULL(SUM(order_total),0) FROM uc_orders WHERE uid=u.uid) -
	IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id IN(SELECT order_id FROM uc_orders WHERE uid=u.uid)),0)
		FROM users u
		WHERE u.uid NOT IN(SELECT uid FROM profile_values WHERE fid=28))";	
	$result = db_query($query);
	if (!$result) {
		$message  = 'Invalid query50: ' . mysql_error() . "\n";
		$message .= 'Whole query: ' . $query;
		drupal_set_message($message);
	}  
	
}
	
	
	
	
	
	
	
	
	