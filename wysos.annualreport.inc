<?php 

function wysos_annual_report_form($form, &$form_state){
	
	$form['filing_name'] = array(
		'#title' => t('Filing Name'),
        '#type' => 'textfield',
        '#size' => '50',
		'#maxlength' => '80',
        '#autocomplete_path' => 'wysos/filing/autocomplete',
        '#required' => TRUE,
		'#description' => t('Enter company name.')
	);
	$year = date('Y');
	$form['ar_year'] = array(
		'#title' => t('AR Year'),
		'#type' => 'select',
		'#options' => array($year-2=>$year-2, $year-1=>$year-1, $year=>$year, $year+1=>$year+1),
		'#default_value' => $year,
		'#required' => TRUE,
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'submit',
	);
	
	return $form;
}
function wysos_annual_report_form_validate($form, &$form_state){
	if(db_query("SELECT COUNT(*) FROM wysosfiling WHERE FILING_NAME=:FILING_NAME AND (STATUS IN('Active','Delinquent') OR (STATUS IN('Inactive - Administratively Dissolved (Tax)','Inactive - Administratively Dissolved (No Agent)') AND DATEDIFF(INACTIVE_DATE,CURRENT_DATE()) < 761) AND FILING_TYPE IN('Profit Corporation','NonProfit Corporation','Limited Liability Company','Limited Partnership', 'Statutory Trust','Registered Limited Liability Partnership'))",array(':FILING_NAME'=>$form_state['values']['filing_name']))->fetchField() == 0){
	 
	 form_set_error('filing_name', 'No company found: '.$form_state['values']['filing_name']);
	}	
}
function wysos_annual_report_form_submit($form,&$form_state){

// Set values based on order type
	include_once('wysos.util.inc');
	
	$row = db_query("SELECT * FROM wysosfiling WHERE FILING_NAME=:FILING_NAME AND (STATUS IN('Active','Delinquent') OR (STATUS IN('Inactive - Administratively Dissolved (Tax)','Inactive - Administratively Dissolved (No Agent)') AND DATEDIFF(INACTIVE_DATE,CURRENT_DATE()) < 761) AND FILING_TYPE IN('Profit Corporation','NonProfit Corporation','Limited Liability Company','Limited Partnership', 'Statutory Trust','Registered Limited Liability Partnership'))", array(':FILING_NAME'=>$form_state['values']['filing_name']))->fetchObject(); 
	
	//print_r($row);
	//die();
	
	//$row = db_fetch_array($result);
	$filing_date = getdate(strtotime($row->FILING_DATE));
	$ar_due = date('F j, Y',mktime(0,0,0,$filing_date['mon'],1,$form_state['values']['ar_year']));

	$GLOBALS['ar_year'] = $form_state['values']['ar_year'];
	$GLOBALS['ar_due'] = $ar_due;
	$GLOBALS['filing_num'] = $row->FILING_NUM;
	$GLOBALS['formation_locale'] = $row->FORMATION_LOCALE;
	$GLOBALS['filing_name'] = $row->FILING_NAME;

	// Open the OpenOffice Template
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');
	
	$doc_name = getFilingTypeAbbrev($row->FILING_TYPE) . '-AR_Form.odt';

	$TBS = new clsTinyButStrong;
	$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

	$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
	//$TBS->Show(TBS_OUTPUT, $form_state['values']['ar_year'].' Annual Report-'.$form_state['values']['filing_name'].'.odt');
	$TBS->Show();
}
function wysos_reinstate_form($form, &$form_state){
	$form['filing_name'] = array(
		'#title' => t('Filing Name'),
		'#type' => 'textfield',
		'#size' => '50',
		'#maxlength' => '80',
		'#autocomplete_path' => 'wysos/reinstate/autocomplete',
		'#required' => TRUE,
		'#description' => t('Enter company name.'),
	);
	$form['user_name'] = array(
		'#title' => t('User Name'),
		'#type' => 'textfield',
		'#size' => '30',
		'#maxlength' => '60',
		'#autocomplete_path' => 'user/autocomplete',
		'#default_value' => '',
		'#description' =>t('Please enter user name'),
	);
	$form['email'] = array(
		'#title' => t('Send to email'),
		'#type' => 'textfield',
		'#size' => '60',
		'#maxlength' => '256',
		'#autocomplete_path' => 'wysos/email/autocomplete',
		'#required' => TRUE,
	);
	$form['create_invoice'] = array(
		'#title' => t('Create Invoice'),
		'#type' => 'checkbox',
	);
	$form['email_invoice'] = array(
		'#title' => t('Email invoice to customer'),
		'#type' => 'checkbox',
	);
	$form['first_name'] = array(
		'#title' => t('Customer First Name'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '256',
		'#default_value' =>'',
	);
	$form['last_name'] = array(
		'#title' => t('Customer Last Name'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '256',
		'default_value' => '',
	);
	$form['street1'] = array(
		'#title' => t('Address'),
		'#type' => 'textfield',
		'#size' => '60',
		'#maxlength' => '256',
		'#default_value' => '',
	);
	$form['street2'] = array(
		'#type' => 'textfield',
		'#size' => '60',
		'#maxlength' => '256',
		'#default_value' => '',
	);
	$form['city'] = array(
		'#title' => t('City'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '256',
		'#default_value' => '',
	);
	if (isset($_POST['country'])) {
    	$country_id = intval($_POST['country']);
  	} else {
    	$country_id = NULL;
  	}
	
	$countries = db_query("SELECT country_id,country_name FROM uc_countries")->fetchAll();
	
	$country_list = array(); 
	foreach($countries as $country){
		$country_list[$country->country_id] = $country->country_name;
	}
	$form['country'] = array(
		'#type' => 'select',
		'#options' => $country_list,
		'#title' => t('Country'),
		'#id' => 'tcountry', 
	);
	//$form['payment']['billing_zone'] = uc_zone_select(uc_get_field_name('zone'), NULL, NULL, $country_id);
	$zones = db_query("SELECT zone_id, zone_name FROM uc_zones")->fetchAll();
	$zone_list = array();
	foreach($zones as $zone){
		$zone_list[$zone->zone_id] = $zone->zone_name;
	}
	$form['zone'] = array(
		'#type' => 'select',
		'#options' => $zone_list,
		'#title' => t('State/Province'),
		'#id' => 'tzone', 
	);
	$form['zone_load'] = array(
		'#type' => 'item',
	);
	$form['postal_code'] = array(
		'#title' => t('Zip/Postal code'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '40',
		'#default_value' => '',
	);
	$form['phone'] = array(
		'#title' => t('Phone Number'),
		'#type' => 'textfield',
		'#size' => '20',
		'#maxlength' => '20',
		'#default_value' => '',
	);
	$form['ra_fee'] = array(
		'#title' => t('Registered Agent Fee'),
		'#type' => 'textfield',
		'#size' => '6',
		'#maxlength' => '6',
		'#default_value' => '99.00'
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Submit',
	);
	
	return $form;
}
function wysos_reinstate_form_validate($form, &$form_state)
{
	if(db_query("SELECT COUNT(*) FROM wysosfiling WHERE FILING_NUM = :FILING_NUM 
	AND (STATUS IN('Inactive - Administratively Dissolved (Tax)','Inactive - Administratively Dissolved (No Agent)') 
	AND DATEDIFF(INACTIVE_DATE,CURRENT_DATE()) < 761)
	AND FILING_TYPE IN('Profit Corporation','NonProfit Corporation','Limited Liability Company','Limited Partnership','Statutory Trust', 'Registered Limited Liability Partnership')",array(':FILING_NUM'=>$form_state['values']['filing_name']))->fetchField() == 0){
		form_set_error('filing_name','No company found: '.$form_state['values']['filing_name']);
	}
}
function wysos_reinstate_form_submit($form, &$form_state){
	return wysos_reinstate($form_state['values']['filing_name']);
}

/**
 * Reinstate CID
 */
function wysos_reinstate($cid){
	wysos_scrape_registered_agent($cid);
	$download_url = 'wysos/download';
	$download_path = $_SERVER['DOCUMENT_ROOT'].'/wra_dl';
	
	//Set values based on order type
	include_once('wysos.util.inc');
	
	$row = db_query("SELECT * FROM wysosfiling WHERE FILING_NUM= :FILING_NUM
		AND (STATUS IN('Inactive - Administratively Dissolved (Tax)','Inactive - Administratively Dissolved (No Agent)') 
		AND DATEDIFF(INACTIVE_DATE,CURRENT_DATE()) < 761)
		AND FILING_TYPE IN('Profit Corporation','NonProfit Corporation','Limited Liability Company','Limited Partnership',
		'Statutory Trust','Registered Limited Liability Partnership')", array(':FILING_NUM' => $cid))->fetchObject();
		
	if(!$row)
	{
		// Not a valid CID
		drupal_set_message('Invalid Reinstatement Entity');
		return;
	}
	$filing_date = new DateTime($row->FILING_DATE); 
//	$filing_date = new DateTime('Y-m-!d H:i:s', $row['FILING_DATE']); 
	$inactive_date = new DateTime($row->INACTIVE_DATE);
//	$inactive_date = new DateTime('Y-m-!d H:i:s', $row['INACTIVE_DATE'] );
	
	// Get Last Annual Report Filed
	$last_ar_yr = db_query("SELECT MAX(FILING_YEAR) FROM wysosfiling_annual_report WHERE FILING_ID=:FILING_ID",array(':FILING_ID' => $row->FILING_ID))->fetchField();
	
	if( empty($last_ar_yr) )
		$last_ar_yr = $filing_date->format('Y');
//	$ar_due = new DateTime('Y-m-!d',(++$last_ar_yr).'-'.$filing_date->format('m').'-1');
	$ar_due = new DateTime((++$last_ar_yr).'-'.$filing_date->format('m').'-1');
	$date_now = new DateTime("now");
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');
	
	while($ar_due < $date_now){
		$base_file = $row->FILING_NAME.'-AR-'.$last_ar_yr.'.odt';
		$save_file = $download_path.$base_file;
		$doc_name = getFilingTypeAbbrev($row->FILING_TYPE) . '-AR_Form.odt';

		$GLOBALS['ar_year'] = $last_ar_yr;
		$GLOBALS['ar_due'] = $ar_due->format('F j, Y');
		$GLOBALS['filing_num'] = $row->FILING_NUM;
		$GLOBALS['formation_locale'] = $row->FORMATION_LOCALE;
		$GLOBALS['filing_name'] = $row->FILING_NAME;
	
		$TBS = new clsTinyButStrong;
		$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
	
		$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
		$TBS->Show(OPENTBS_FILE, $save_file);
		$key = $ar_due->format('Y').' Annual Report';
		$dl_file[$key] = $base_file;
		$ar_due = new DateTime((++$last_ar_yr).'-'.$filing_date->format('m').'-1');
//		$TBS->__destruct();
//		date_add($ar_due, date_interval_create_from_date_string('1 year'));
//		$ar_due->add(new DateInterval('P1Y'));
	}
	global $user;
	
	$r_address = db_query("SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
			FROM users
			INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
			INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
			WHERE users.uid = :uid", array(':uid'=>$user->uid))->fetchObject();
			
	// Make Reinstatement PDF
	switch(getFilingTypeAbbrev($row->FILING_TYPE))
	{
		case 'LLC' :
			$pdf_template='http://my.wyomingregisteredagent.com/public/wyforms/LLC-ReinstatementForm.pdf';
			$data['Text5'] = 'Manager';
			break;
			
		case 'CORP' :
			$pdf_template='http://my.wyomingregisteredagent.com/public/wyforms/P-ReinstatementForm.pdf';
			break;
	}
	// we will use this array to pass to the createFDF function
	
	$data['Text1'] = $row->FILING_NAME;
	$data['Text2'] = $inactive_date->format('m/d/Y');
	$data['Text6'] = $r_address->first_name .' '. $r_address->last_name;
	$data['Text7'] = $r_address->phone;
	$data['Text8'] = $r_address->email;
	
	require_once 'fdf.php';
	// file name will be <the current timestamp>.fdf
	$base_file = $row->FILING_NAME.'-Reinstatement';
	$fdf_file=$base_file.'.fdf';
	$pdf_file=$base_file.'.pdf';
	
	// the directory to write the result in
	$fdf_dir=$download_path;
	
	// need to know what file the data will go into
	$pdf_doc=$fdf_dir.'/'.$pdf_file;
	$fdf_doc=$fdf_dir.'/'.$fdf_file;
	
	// generate the file content
	$fdf_data=pdf_fdf::Make($pdf_template,$data);
	// this is where you'd do any custom handling of the data
	// if you wanted to put it in a database, email the
	// FDF data, push ti back to the user with a header() call, etc.
	// write the file out
	if($fp=fopen($fdf_doc,'w')){
		fwrite($fp,$fdf_data,strlen($fdf_data));
	}else{
		die('Unable to create file: '.$fdf_dir.'/'.$fdf_file);
	}
	fclose($fp);	
	
//	pdf_fdf::FDF2PDF($fdf_dir.'/'.$fdf_file,$pdf_doc);
	exec("pdftk \"$pdf_template\" fill_form \"$fdf_doc\" output \"$pdf_doc\"");
	unlink($fdf_dir.'/'.$fdf_file);

	$dl_file['Reinstatement Form'] = $pdf_file;
	// Include download library
	include_once('wysos.download.inc');

	$did = wysos_download_add('Reinstatement for '.$row->FILING_NAME, $dl_file);
	
	drupal_goto($download_url.'/'.$did);
		
}

function wysos_scrape_registered_agent($cid)
{
	$url = "https://wyobiz.wy.gov/Business/FilingDetails.aspx?FilingNum=".$cid;
	$raw = file_get_contents($url);

	$newlines = array("\t","\n","\r","\x20\x20","\0","\x0B");
	$content = str_replace($newlines, "", html_entity_decode($raw));

	$start = strpos($content,'<span id="ctl00_contentMain_txtAgentName">');
	$end = strpos($content,'</span>',$start);
	$ra_name = trim(substr($content,$start,$end-$start));
	
	$start = strpos($content,'<span id="ctl00_contentMain_txtAgentAddress">');
	$end = strpos($content,'</span>',$start);
	$raw_address = substr($content,$start,$end-$start);
	$address = trim(str_replace("<br/>", ", ", $raw_address));

	$retArray = array();
	$retArray['ra_name'] = $ra_name;
	$retArray['ra_address'] = $address;
	
	return $retArray;
}

function filesystem_base_path()
{
    if(!isset($GLOBALS['filesystem_base_path']))
    {
      $search = "includes".DIRECTORY_SEPARATOR."bootstrap.inc"; 
	  // Walk directories up until we find the $search-path (wich is relative to root)
      for($path=dirname(__FILE__); !file_exists($path.DIRECTORY_SEPARATOR.$search); $path.= DIRECTORY_SEPARATOR."..")
      {
        // no need to do anything
      }
      // store the path if it one was found
      $GLOBALS['filesystem_base_path'] = realpath($path);
    }
    return $GLOBALS['filesystem_base_path'];
}
