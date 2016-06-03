<?php 

function wysos_suite_next_available(){
	$next_suite = db_query("SELECT MAX(suite_no)+1 FROM {wra_suites}")->fetchField();
	$output = 'Next available suite: '.$next_suite;
	return $output;
}

function wysos_suite_cleanup(){
	// need to work
	/*
		// Delete SIP Users not active
		$result = db_query("DELETE FROM fed_sip.sip_buddies WHERE name NOT IN(SELECT r.value FROM wyregne_drupal.wra_suite_resources r INNER JOIN wyregne_drupal.wra_suites s ON r.suite_id = s.suite_id
			WHERE s.status = 'active' AND r.resource_type='sip_user')"); 	
			
		// Delete Resources for Non-existent suites
		$result = db_query("DELETE FROM wra_suite_resources WHERE suite_id NOT IN(SELECT suite_id FROM wra_suites)");
		
		// Delete Suspended
		//DELETE FROM wra_suite_resources WHERE suite_id NOT IN(SELECT suite_id FROM wra_suites WHERE status IN('active','order_entry');
	*/
		$result = db_query("DELETE FROM wra_suite_resources WHERE suite_id NOT IN (SELECT suite_id FROM wra_suites)");
} 

function wysos_mail_override_form($form, &$form_state,$company_id = NULL){
	if($company_id){
		$company_name = db_query("SELECT company_name FROM {wra_companies} WHERE company_id = :company_id",array(':company_id'=>$company_id))->fetchField();
		drupal_set_title('Override Mail for '.$company_name);
		$form['company'] = array(
			'#title' => t('Company'),
			'#type' => 'textfield',
			'#size' => 60,
			'#default_value' => $company_name,
			'#disabled' => TRUE,
		);
	}
	else{
		drupal_set_title('Override Mail');
		$form['company'] = array(
			'#title' => t('Company'),
			'#type' => 'textfield',
			'#size' => 60,
			'#autocomplete_path' => 'wysos/autocomplete/all_companies',
			'#required' => TRUE,
		);
	}
	$form['mail-action'] = array(
		'#title' => t('Mail Action'),
		'#type' => 'select',
		'#options' =>array(
			'' => '',
			'fwd' => 'Forward (fwd)',
			'fwd2incorp' => 'Forward to INCORP (fwd2incorp)',
			'fwd229dollaragent' => 'Forward to 29DollarAgent (fwd229dollaragent)',
			'fwd2primera' => 'Forward to Primera (fwd2primera)',
			'fwd2uscorpagents' => 'Forward to US Corporation Agents (fwd2uscorpagents)',
			'fwd2rals' => 'Forward to RALS (fwd2rals)',
			'fwd2regagentofwy' => 'Forward to Registered Agents of Wyoming (fwd2regagentofwy)',
			'hold' => 'Hold (hold)',
			'hold_over20' => 'Hold Over 20 Pieces (hold_over20)',
			'hold_vosuspended' => 'Hold Virtual Office Suspended (hold_vosuspended)',
			'in_office' => 'In Office (in_office)',
			'rts' => 'Return To Sender (rts)',
			'scan-fwd' => 'Scan then Forward (scan-fwd)',
			'scan' => 'Scan (scan)',
			'unassigned' => 'Unassigned',
			),
		'#default_value' => '',
		'#required' => TRUE,	
	);
	$form['reason'] = array(
		'#title' => t('Reason'),
		'#type' => 'textarea',
		'#required' => TRUE,
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Override'),
	);
	return $form;
}
function wysos_mail_override_form_validate($form, &$form_state){
	if(db_query("SELECT COUNT(*) FROM {wra_companies} WHERE company_name = :company_name",array(':company_name'=>$form_state['values']['company']))
	->fetchField() == 0 ) {
		form_set_error('company','Company name not found');
	}

}


function wysos_mail_override_form_submit($form, &$form_state){
	global $user;
	
	//Get company_id
	$result = db_query("SELECT company_id, mail_action FROM wra_companies WHERE company_name = '".mysql_real_escape_string($form_state['values']['company'])."'")->fetchObject();
	
	//add wra_company_override	
	$result1 = db_insert('wra_company_mail_override')
				->fields(array(
					'company_id' => $result->company_id,
					'comments' => $form_state['values']['reason'],
					'mail_action' => $form_state['values']['mail-action'],
					'created_uid' => $user->uid,
					'created' => REQUEST_TIME,
				))->execute();
	
	// Add wra_company_log
	$result2 = db_insert('wra_company_log')
				->fields(array(
					'company_id' => $result->company_id,
					'log_type' => 'Override Mail Action',
					'uid' => $user->uid,
					'old_value' => $result->mail_action,
					'new_value' => $form_state['values']['mail-action'],
					'created' => REQUEST_TIME,
				))->execute();
				
	//update wra_companies
	$result3 = db_update('wra_companies')->fields(array('mail_action'=>$from_state['values']['mail-action']))->condition('company_id',$result->company_id,'=')->execute();
	
	drupal_set_message('Mail for '.$form_state['values']['company'].' changed from '.$result->mail_action.' to '.$form_state['values']['mail-action'].'.');

}
function wysos_suite_make_config_form($form, &$form_state, $suite_id=NULL){
	drupal_set_title('Program Adapter for Suite '.$suite_id);
	// Load Suites from mysql
	
	$result = db_query("SELECT suite_id, suite_no, company_name FROM wra_suites WHERE status='active' AND service_level IN('silver','gold') ORDER BY company_name")->fetchAll();
	
	$options = array(0=> 'Select Suite');
	foreach($result as $row){
		$options[$row->suite_id] = sprintf('%4d-%-3os', $row->suite_no, $row->company_name);
	}
	$form['suite'] = array(
		'#title' => t('Suite'),
		'#type' => 'select',
		'#options' => $options,
		'#default_value' => ($suite_id) ? $suite_id : 0,
		'#required' => TRUE,
	);
	$form['mac'] = array(
		'#title' => t('VoIP MAC'),
		'#type' => 'textfield',
		'#size' => '12',
		'#maxlength'=> '12',
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Configure Adapter'),
	);	
	return $form;

}
function wysos_suite_make_config_form_validate($form, &$form_state){

}
function wysos_suite_make_config_form_submit($form, &$form_state){
	// Create Grandstream Config text files
	include_once('wysos.vo_util.inc');
	$suite_id = db_query("SELECT suite_id FROM wra_suites WHERE suite_id= :suite_id AND status= 'active' AND
	service_level IN('silver', 'gold')",array(':suite_id' => $form_state['values']['suite']))->fetchField();
	wysos_vo_util_make_config(suite_id);
	
}

function wysos_suite_user_shipment_form($form, &$form_state){
	if(isset($_SERVER['HTTP_USER_AGENT']) && (strpos($_SERVER['HTTP_USER_AGENT'],'MSIE')=== false))
	{
		drupal_goto('ie_required');
		return;
	}
	drupal_set_title('Mail Forwarding Shipment');
	
	$form['user_name'] = array(
		'#title' => t('User Name'),
		'#type' => 'textfield',
		'#size' => '30',
		'#maxlength' => '60',
		'#autocomplete_path' => 'user/autocomplete',
		'#required' => TRUE,
		'#description' => t('Please enter user name'),
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
	);
	return $form;

}
function wysos_suite_user_shipment_form_validate($form, &$form_state){
	if(!user_load_by_name($form_state['values']['user_name'])){
		form_set_error('user_name',t('Invalid User Name.'));
	}
}
function wysos_suite_user_shipment_form_submit($form, &$form_state){
$user = user_load_by_name($form_state['values']['user_name']);
$form_state['redirect'] =  '/wysos/suite/new_shipment/'.$user->uid;
drupal_redirect_form($form_state);

}

function wysos_suite_shipment_form($form, &$form_state, $uid, $type='fwd'){
	// Abort if Not IE
	if(isset($_SERVER['HTTP_USER_AGENT']) && (strpos($_SERVER['HTTP_USER_AGENT'],'MSIE')=== false)){
		drupal_goto('ie_required');
		return;
	}
	$ms_user = user_load($uid);
		
	$name = db_query("SELECT a.aid, a.first_name, a.last_name, a.company, a.street1, a.street2, a.city, a.zone, d.zone_code AS state, a.country, e.country_name, a.postal_code, c.name as user_name 
		FROM uc_addresses a 
		INNER JOIN uc_addresses_defaults b ON b.aid=a.aid AND b.uid=:uid
		INNER JOIN users c ON b.uid=c.uid
		INNER JOIN uc_zones d ON a.zone = d.zone_id
		INNER JOIN uc_countries e ON a.country = e.country_id", array(':uid' => $uid))->fetchObject();
	
	$form_state['storage']['address'] = $name;
	drupal_set_title('Shipment for '.$name->first_name.' '.$name->last_name.' ('.$name->user_name.')');
	$headers = array('Envelope','Received','Company');
	if($type == 'fwd') {
		$result = db_query("SELECT c.company_name, m.filename, m.created 
				FROM wra_company_mail m
				INNER JOIN wra_companies c ON m.company_id = c.company_id
				WHERE c.uid=:uid AND m.shipment_id IS NULL AND m.location='fwd'", array(':uid'=>$uid))->fetchAll();
	}
	$rows = array();
	#pieces = 0;
	foreach($result as $rMail) {
		if(empty($rMail->filename)) {
			$rows[] = array("(no image)",date('M-d', $rMail->created),$rMail->company_name);
		} else {
			$rows[] = array("<a href='/envelopes/".$rMail->filename."'><img src='/envelopes/".$rMail->filename."' height='60'></a>",date('M-d', $rMail->created),$rMail->company_name);
		}
		$pieces++;
	}
	$form['title'] = array(
	'#type' => 'item',
	'#prefix' => theme('table', array('header'=>$headers, 'rows'=>$rows)).'<br />' 
	);
	$form['type'] = array(
		'#type' => 'hidden',
		'#value' => $type
	);
	$form['uid'] = array(
		'#type' => 'hidden',
		'#value' => $uid
	);

	if(!empty($mf_user->profile_shipping_notes))
	{
		$form['shipping_notes'] = array(
			'#title' => t("Shipping Instuctions"),
			'#type' => 'item',
			'#value' => $mf_user->profile_shipping_notes
		);
	
	}
	if(!empty($mf_user->profile_fedex_account))
	{
		$form['shipping_notes'] = array(
			'#title' => t("FedEx Account"),
			'#type' => 'item',
			'#value' => $mf_user->profile_fedex_account
		);
	}
	if(!empty($mf_user->profile_ups_account))
	{
		$form['shipping_notes'] = array(
			'#title' => t("UPS Account"),
			'#type' => 'item',
			'#value' => $mf_user->profile_ups_account
		);
	}
	$form['weight_lbs'] = array(
		'#title' => t('Weight (lbs)'),
		'#type' => 'textfield',
		'#size' => '3',
		'#maxlength' => '3',
		'#default_value' => '0',
		'#required' => TRUE
	);
	$form['weight_oz'] = array(
		'#title' => t('Weight (oz)'),
		'#type' => 'textfield',
		'#size' => '5',
		'#maxlength' => '5',
		'#required' => TRUE,
	);
	$form['form_factor'] = array(
		'#title' => t('Package'),
		'#type' => 'select',
		'#options' => array(1=>'Envelope', 10=>'Thick Envelope', 4=>'Flat Rate Envelope'),
		'#default_value' => 1,
		'#required' => TRUE
	);
	
	$form['form_factor'] = array(
		'#type' => 'hidden'
	);
	$form['mail_class'] = array(
		'#type' => 'hidden'
	);
	$form['cost_code'] = array(
		'#type' => 'hidden',
		'#value' => isset($cost_code) ? $cost_code:'',
	);
	$form['weight_total'] = array(
		'#type' => 'hidden',
	);
	$form['postage'] = array(
		'#type' => 'hidden',
	);
	$form['tracking'] = array(
		'#type' => 'hidden',
	);
	$form['media_id'] = array(
		'#type' => 'hidden',
	);
	$form['pieces'] = array(
		'#title' => t('Pieces'),
		'#type' => 'textfield',
		'#size' => '2',
		'#maxlength' => '3',
		'#default_value' => isset($pieces)?$pieces:'',
		'#required' => TRUE
	);
	$recipient = "";
	if(!empty($name->company))
		$recipient = $name->company. "\\r\\n";
	$recipient .= 	$name->first_name . ' '. $name->last_name . '\\r\\n';
	$recipient .=  $name->street1 . "\\r\\n";
	if(!empty($name->street2))
		$recipient .= $name->street2. "\\r\\n";
	$recipient .= $name->city . ', ' . $name->state . ' ' . $name->postal_code;
	if($name->country_name != 'United States')
		$recipient .= "\\r\\n" . $name->country_name;
//			print_job.PrinterName = "Xerox Phaser 6180MFP-D PCL 6";
//			print_job.MailpieceWeight = document.wysos-stamps-mf-shipment-form.edit-weight-oz.value;
//			print_job.AddRecipient('.$recipient.');
	
	drupal_add_js('
		function print_label()
		{
			$total_oz = parseFloat((document.getElementById("edit-weight-lbs").value * 16)) + parseFloat(document.getElementById("edit-weight-oz").value);
			var pdk = new ActiveXObject("StampsDotCom.PDK5");
			var internet_postage = pdk.InternetPostage3;
			var print_job = pdk.CreatePrintJob5();
			if($total_oz > 13) {
				// Priority Mail
				print_job.MailClass = 1;
				print_job.PrinterName = "DYMO 30387";
				print_job.MediaID = 75; // Dymo 30387
				print_job.MailpieceFormFactor = 4;
				print_job.AppName = "my.wyomingregisteredagent.com";
				print_job.ReturnAddress = "17th & Central Executive Suites\\r\\n1623 Central Ave.\\r\\nCheyenne, WY 82001";
				print_job.MailpieceWeight = $total_oz;
				print_job.ReferenceCode = document.getElementById("edit-cost-code").value;				
				print_job.AddRecipient("'.$recipient.'");
				internet_postage.PrintShippingLabelEx(1, print_job);
			} else {
				// First Class
				print_job.MailClass = 0;
				print_job.PrinterName = "DYMO 30383";
				print_job.MediaID = 11; // Dymo 30383
				print_job.MailpieceFormFactor = 1;
				print_job.AppName = "my.wyomingregisteredagent.com";
				print_job.ReturnAddress = "17th & Central Executive Suites\\r\\n1623 Central Ave.\\r\\nCheyenne, WY 82001";
				print_job.MailpieceWeight = $total_oz;
				print_job.ReferenceCode = document.getElementById("edit-cost-code").value;				
				print_job.AddRecipient("'.$recipient.'");
				internet_postage.PrintPostageEx(1, print_job);
			}
			var print_results = print_job.PrintResults;
			if (print_results.Count > 0)
			{
				var result = print_results(0);
				document.getElementById("edit-form-factor").value = result.MailPieceFormFactor;
				document.getElementById("edit-mail-class").value = result.MailClass;
				document.getElementById("edit-postage").value = result.PostageAmount;
				document.getElementById("edit-weight-total").value = result.MailPieceWeight / 100;
				document.getElementById("edit-tracking").value = result.TrackingNumber;
				document.getElementById("edit-media-id").value = print_job.MediaID;
				delete pdk;
				return true;
			} else {
				delete pdk;
				return false;
			}
		}
', 'inline');
	$form['print'] = array (
		'#type' => 'submit',
		'#value' => t('Print Postage'),
		'#attributes' => array('onclick' => 'return print_label();')
	);
	return $form; 
}
function wysos_suite_shipment_form_validate($form, &$form_state) {
	if($form_state['values']['weight_total'] <= 0)
		form_set_error('weight_total','Weight must be more than 0.');
	if($form_state['values']['postage'] <= 0)
		form_set_error('postage','Postage must be more than 0.');
}
function wysos_suite_shipment_form_submit($form, &$form_state) {
//drupal_set_message('form_state: '.print_r($form_state, true));

	$mf_user = user_load($form_state['values']['uid']);
	global $user;	
		$result = db_insert('wra_user_shipment')
				->fields(array(
					'to_uid' => $form_state['values']['uid'],
					'aid' => $form_state['storage']['address']['aid'],
					'first_name' => $form_state['storage']['address']['first_name'],
					'last_name' => $form_state['storage']['address']['last_name'],
					'phone' => $form_state['storage']['address']['phone'],
					'company' => $form_state['storage']['address']['company'],
					'street1' => $form_state['storage']['address']['street1'],
					'street2' => $form_state['storage']['address']['street2'],
					'city' => $form_state['storage']['address']['city'],
					'zone' => $form_state['storage']['address']['zone'],
					'postal_code' => $form_state['storage']['address']['postal_code'],
					'country' => $form_state['storage']['address']['country'],
					'weight_oz' => $form_state['values']['weight_total'],
					'pieces' => $form_state['values']['pieces'],
					'postage' => $form_state['values']['postage'],
					'tracking_no' => $form_state['values']['tracking'],
					'created_uid' => REQUEST_TIME,
					'sent' => $user->uid,
				))->execute();
		
	if (!$result) {
		return;
	}  
	$shipment_id = $result;
	
	if($form_state['values']['type'] == 'fwd') {	
		db_query("UPDATE wra_company_mail m
		INNER JOIN wra_companies c ON m.company_id=c.company_id 
		SET m.shipment_id=:shipment_id 
		WHERE c.uid=:uid AND m.shipment_id IS NULL AND m.location='fwd'", array(':shipment_id'=>$shipment_id, ':uid'=>$mf_user->uid))->execute();
	}
	
	setlocale(LC_MONETARY, 'en_US');
	
// Email Customer that mail was sent
	drupal_set_message("Shipment for ".money_format('%n', $form_state['values']['postage'])." added.");
	drupal_redirect_form($form,'wysos/suite/user_shipment');
}
function wysos_suite_mail_lookup_form($form,&$form_state){
	$form['company_name'] = array(
		'#title' => t('Company Name'),
		'#type' => 'textfield',
		'#size' => '110',
		'#maxlength' => '130',
		'#autocomplete_path' => 'wysos/mail/lookup2/autocomplete',
		'#description' => t('Enter company name'),
	);
	return $form;
}
function wysos_suite_filing_user_form($form, &$form_state) {
	$form['company_name'] = array(
		'#title' => t('Company Name'),
        '#type' => 'textfield',
        '#size' => '110',
		'#maxlength' => '130',
        '#autocomplete_path' => 'wysos/mail/lookup/autocomplete',
		'#description' => t('Enter company name.')
	);

	$form['submit-company-name'] = array(
		'#type' => 'submit',
		'#value' => 'By Company'
		); 
	$form['full_name'] = array(
		'#title' => t('Person Name'),
        '#type' => 'textfield',
        '#size' => '110',
		'#maxlength' => '130',
        '#autocomplete_path' => 'wysos/full_name/autocomplete',
		'#description' => t('Enter last name, first name.')
	);

	$form['submit-full-name'] = array(
		'#type' => 'submit',
		'#value' => 'By Name'
		); 
	$form['suite_no'] = array(
		'#title' => t('Suite No'),
        '#type' => 'textfield',
        '#size' => '110',
		'#maxlength' => '130',
        '#autocomplete_path' => 'wysos/suite_no/autocomplete',
	);

	$form['submit-suite-no'] = array(
		'#type' => 'submit',
		'#value' => 'By Suite No'
		); 

	return ($form);
		
}
function wysos_suite_filing_user_form_submit($form, &$form_state)
{
	global $user;

	switch ($form_state['clicked_button']['#id']) {
		case 'edit-submit-company-name' :
			$result = db_query("SELECT c.company_name, c.company_id, c.uid, s.suite_id, u.name AS user_name, u.mail as email, c.mail_action,
				IF(UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')) > UNIX_TIMESTAMP(),
				  (SELECT COUNT(company_mail_id) FROM wra_company_mail
					WHERE company_id=c.company_id 
					AND created > UNIX_TIMESTAMP(CONCAT(YEAR(NOW())-1,'-',LPAD(MONTH(c.filing_date),2,'0'),'-01'))),
				(SELECT COUNT(company_mail_id) FROM wra_company_mail
					WHERE company_id=c.company_id 
					AND created > UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')))) AS pieces_year,
				IF(UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')) > UNIX_TIMESTAMP(),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW())-1,'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')))
				 AS start_dt,
				IF(UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')) > UNIX_TIMESTAMP(),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW())+1,'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')))
				 AS end_dt
				
				FROM  wra_companies c 
				LEFT JOIN wra_suites s ON c.suite_id = s.suite_id
				LEFT JOIN users u ON c.uid = u.uid
				WHERE c.company_name = :company_name  AND (c.scraped_ra_id=249 OR c.suite_id IS NOT NULL)", array(':company_name'=>$form_state['values']['company_name']))->fetchObject();
			break;
		
		case 'edit-submit-full-name' :
			$result = db_query("SELECT c.company_name, c.company_id, c.uid, s.suite_id, u.name AS user_name, u.mail as email, c.mail_action,
				IF(UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')) > UNIX_TIMESTAMP(),
				  (SELECT COUNT(company_mail_id) FROM wra_company_mail
					WHERE company_id=c.company_id 
					AND created > UNIX_TIMESTAMP(CONCAT(YEAR(NOW())-1,'-',LPAD(MONTH(c.filing_date),2,'0'),'-01'))),
				(SELECT COUNT(company_mail_id) FROM wra_company_mail
					WHERE company_id=c.company_id 
					AND created > UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')))) AS pieces_year,
				IF(UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')) > UNIX_TIMESTAMP(),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW())-1,'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')))
				 AS start_dt,
				IF(UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')) > UNIX_TIMESTAMP(),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW()),'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')),
				  UNIX_TIMESTAMP(CONCAT(YEAR(NOW())+1,'-',LPAD(MONTH(c.filing_date),2,'0'),'-01')))
				 AS end_dt
			FROM  wra_companies c
			LEFT JOIN wra_suites s ON c.suite_id = s.suite_id
			INNER JOIN users u ON c.uid = u.uid
			INNER JOIN uc_addresses a ON a.uid=c.uid
			WHERE CONCAT(a.last_name,', ',a.first_name)=:full_name AND (c.scraped_ra_id=249 OR c.suite_id IS NOT NULL OR c.mail_action IS NOT NULL)", array(':full_name'=>$form_state['values']['full_name']))->fetchObject();
			break;

		case 'edit-submit-suite-no' :
			$result = db_query("SELECT c.company_name, c.company_id, c.uid, s.suite_id, u.name AS user_name, u.mail as email, c.mail_action,
				'0' AS pieces_year
				FROM wra_suites s
				INNER JOIN wra_companies c ON s.company_id = c.company_id
				INNER JOIN users u ON c.uid = u.uid
				WHERE s.suite_no = :suite_no  AND (c.scraped_ra_id=249 OR c.suite_id IS NOT NULL)", array(':suite_no' => $form_state['values']['suite_no']))->fetchObject();
			break;
	}
	$rowCompany = $result;
	if(empty($rowCompany->suite_id) AND $rowCompany->pieces_year > 20) {
		// Over limit - send to HOLD bins	
		require_once 'mail_attachment.php';
		$from = 'mail@wyomingregisteredagent.com';
		$to_email = $rowCompany->email;
//		$to_email = 'mirroredshades@gmail.com';
		
		$body =  "Hello,<br /><p>We recently received mail for your company ".$rowCompany->company_name.". Since you are over the 20 piece per year mail limit for ".$rowCompany->company_name.", we will be unable to forward the mail without the purchase of a mail forwarding package. Three packages are available:</p>";
		$body .= "<p>The <b>Bronze Virtual Office</b> includes a Wyoming street address with suite number and weekly mail forwarding. Postage is included for up to 100 letters per month. The bronze virtual office is $29 monthly or $295 per year.<br />";
		$body .= "<p>The <b>Silver Virtual Office</b> includes a Wyoming street address with suite number and weekly mail forwarding (postage for 100 letters per month is included). You also receive a Wyoming 307 area code phone number with a Voice over IP (VoIP) adapter, voicemail with email forwarding, a separate Wyoming fax number with email forwarding, and unlimited long distance in the U.S. and Canada. The silver virtual office is $49 monthly or $495 per year.<br />";
		$body .= "<p>The <b>Gold Virtual Office</b> includes a Wyoming street address with suite number and weekly mail forwarding (postage for 100 letters per month is included). You also receive a Wyoming 307 area code phone number with a live receptionist that answers in your company name during business hours, voicemail with email forwarding, a separate Wyoming fax number with email forwarding, and unlimited call transfer to any phone number in the U.S. and Canada. The gold virtual office is $79 monthly or $795 per year.</p>";
		$body .= "<p>You can order online using any major credit card by going to: <a href='https://my.wyomingregisteredagent.com/wysos/virtual_office/order'>https://my.wyomingregisteredagent.com/wysos/virtual_office/order</a>. You can also order by phone by calling (307) 637-5151 and pressing 1 for sales. Thank you.</p>";
		// INSERT Company Mail
		$unique_id = sha1(uniqid('',true));
		/*
		db_query("INSERT INTO wra_company_mail (unique_id, company_id, creator_uid, location, bin, created) VALUES ('%s', %d, %d, 'hold', '%s', UNIX_TIMESTAMP())", 
				$unique_id, $rowCompany['company_id'], $user->uid, strtoupper(substr($rowCompany['user_name'],0,1)));
		*/		
		db_insert('wra_company_mail')
				->fields(array(
					'unique_id' => $unique_id,
					'company_id' => $rowCompany->company_id,
					'creator_uid' => $user->uid,
					'location' => 'hold',
					'bin' => strtoupper(substr($rowCompany->user_name,0,1)),
					'created' => REQUEST_TIME,
				))->execute();		
		
		
		// Update Company
		/*db_query("UPDATE wra_companies SET mail_action='hold_over20' WHERE company_id = %d", $rowCompany['company_id']);*/
		
		db_update('wra_companies')
				->fields(array(
					'mail_action' => 'hold_over20',	
				))->condition('company_id',$rowCompany->company_id)->execute();	
		
		mail_attachment($from,$to_email,"Mail overage for ".$rowCompany->company_name, $body, array());
		drupal_set_message('Mail for '.$rowCompany->company_name.' - Place in <b>HOLD '.strtoupper(substr($rowCompany->user_name,0,1)).'</b> for user <b>'.$rowCompany->user_name.'</b>');
		
	}



	switch($rowCompany->mail_action) {
		case 'rts' :
			drupal_set_message('Mail for '.$rowCompany->company_name.' - Mark this mail as <b>RETURN TO SENDER</b>');
			drupal_goto('wysos/suite/mail');
			break;
			
		case 'unassigned' :
			drupal_set_message('Mail for '.$rowCompany->company_name.' - Mark this mail as <b>UNASSIGNED</b>');
			drupal_goto('wysos/suite/mail');
			break;

		case 'scan-fwd' :
			// Redirect to Scan In Page
			$unique_id = sha1(uniqid('',true));
			/*
			db_query("INSERT INTO wra_company_mail (unique_id, company_id, creator_uid, created) VALUES ('%s', %d, %d, UNIX_TIMESTAMP())", 
					$unique_id, $rowCompany['company_id'], $user->uid);
			*/
			
			db_insert('wra_company_mail')
				->fields(array(
					'unique_id' => $unique_id,
					'company_id' => $rowCompany->company_id,
					'creator_uid' => $user->uid,
					'created' => REQUEST_TIME,
				))->execute();
					
			drupal_goto('wysos/suite/scan-envelope/'.$unique_id);
			break;
			
		case 'fwd' 	:
			$unique_id = sha1(uniqid('',true));
			/*db_query("INSERT INTO wra_company_mail (unique_id, company_id, creator_uid, created, location, bin) VALUES ('%s', %d, %d, UNIX_TIMESTAMP(), 'fwd', '%s')", 
					$unique_id, $rowCompany['company_id'], $user->uid, strtoupper(substr($rowCompany['user_name'],0,1)));
			*/
			
			db_insert('wra_company_mail')
				->fields(array(
					'unique_id' => $unique_id,
					'company_id' => $rowCompany->company_id,
					'creator_uid' => $user->uid,
					'created' => REQUEST_TIME,
					'location' => 'fwd',
					'bin' => strtoupper(substr($rowCompany->user_name,0,1)),
				))->execute();
				
			drupal_set_message('Mail for '.$rowCompany->company_name.' - Place in FORWARD '.strtoupper(substr($rowCompany->user_name,0,1)));
			drupal_goto('wysos/suite/mail');
			break;
			
		case 'scan' :
			// Redirect to Scan In Page
			$unique_id = sha1(uniqid('',true));
			/*
			db_query("INSERT INTO wra_company_mail (unique_id, company_id, creator_uid, created) VALUES ('%s', %d, %d, UNIX_TIMESTAMP())", 
					$unique_id, $rowCompany['company_id'], $user->uid);
			*/
			db_insert('wra_company_mail')
				->fields(array(
					'unique_id' => $unique_id,
					'company_id' => $rowCompany->company_id,
					'creator_uid' => $user->uid,
					'created' => REQUEST_TIME,
				))->execute();		
					
			drupal_goto('wysos/suite/scan-envelope/'.$unique_id);
			break;
			
		case 'hold' :
		case 'hold_vosuspended' :
		case 'hold_over20' :
			$unique_id = sha1(uniqid('',true));
			/*			
			db_query("INSERT INTO wra_company_mail (unique_id, company_id, creator_uid, created, location, bin) VALUES ('%s', %d, %d, UNIX_TIMESTAMP(), 'hold', '%s')", 
					$unique_id, $rowCompany['company_id'], $user->uid, strtoupper(substr($rowCompany['user_name'],0,1)));
			*/		
			db_insert('wra_company_mail')
				->fields(array(
					'unique_id' => $unique_id,
					'company_id' => $rowCompany->company_id,
					'creator_uid' => $user->uid,
					'created' => REQUEST_TIME,
					'location' => 'fwd',
					'bin' => strtoupper(substr($rowCompany->user_name,0,1)),
				))->execute();		
			drupal_set_message('Mail for '.$rowCompany->company_name.' - Place in <b>HOLD '.strtoupper(substr($rowCompany->user_name,0,1)).'</b> for user <b>'.$rowCompany->user_name.'</b>');
			drupal_goto('wysos/suite/mail');
			break;
	}
}
function wysos_suite_scan_envelope_form($form, &$form_state, $unique_id){
	/*if(ip_address() != '72.175.205.202'){
		drupal_access_denied();
		return;
	}*/
	$rowCompany = db_query("SELECT c.company_name, c.company_id, m.company_mail_id, u.name AS user_name
FROM wra_company_mail m
INNER JOIN wra_companies c ON m.company_id=c.company_id
INNER JOIN users u ON c.uid=u.uid
WHERE m.unique_id = :unique_id", array(':unique_id'=>$unique_id))->fetchObject();
	if($rowCompany){
	$form_state['storage']['company_mail_id'] = $rowCompany->company_mail_id;
	drupal_set_title('Scan in envelope for '.$rowCompany->company_name.' ('.$rowCompany->user_name.')');
	
	$form['company_name'] = array(
		'#type' => 'hidden',
		'#value' => $rowCompany->company_name );
	$form['company_id'] = array(
		'#type' => 'hidden',
		'#value' => $rowCompany->company_id );
	$form['company_mail_id'] = array(
		'#type' => 'hidden',
		'#value' => $rowCompany->company_mail_id );
	$form['unique_id'] = array(
		'#type' => 'hidden',
		'#value' => $unique_id );
	$form['twain_control'] = array(
		'#value' => "<EMBED
			TYPE='Application/DynamicWebTwain-Plugin'
			WIDTH='450'
			HEIGHT='300'
			PLUGINSPAGE='/public/DynamicWebTWAINPlugIn.msi'>
		</EMBED>",
	);
	$form['form_factor'] = array(
		'#title' => 'Size',
		'#type' => 'select',
		'#options' => array('' => '',
				'envelope#10' => '#10 Envelope',
				'envelope6x9' => '6x9 Envelope',
				'envelope9x12' => 'Full Page' ),
		'#default' => '',
		'#attributes' => array('onchange' => 'SelectSize(); return false;'),
		'#disabled' => true
	);
	$form['scan'] = array(
		'#type' => 'button',
		'#value' => 'Scan',
		'#attributes' => array('onclick' => 'AcquireImage(); return false;'),
		);
	$form['upload'] = array(
		'#type' => 'submit',
		'#value' => 'Upload',
		'#attributes' => array('onclick' => 'Upload(); return false;'),
		'#disabled' => true
		);
	$form['save'] = array(
		'#type' => 'submit',
		'#value' => 'Save',
		'#disabled' => true
		);
	$form['cancel'] = array(
		'#type' => 'submit',
		'#value' => 'Cancel'
		);
	drupal_add_js(drupal_get_path('module', 'wysos') . '/scan-envelope.js');
		
	return $form;
}
}
function wysos_suite_scan_envelope_form_submit($form, &$form_state) {
//	watchdog('inside', 'clicked: '.$form_state['clicked_button']['#id']);
//	drupal_set_message('clicked: '.$form_state['clicked_button']['#id']);
	switch ($form_state['clicked_button']['#id']) {
		case 'edit-save' :
			// Send Email
			$rCompany = db_query("SELECT c.*, u.name AS user_name FROM wra_companies c 
					LEFT JOIN users u ON c.uid = u.uid
					WHERE c.company_id=:company_id", 
					array(':company_id'=>$form_state['values']['company_id']))->fetchObject();
			//$rCompany = db_fetch_array($result);
			
			switch($rCompany->mail_action) {
				case 'scan-fwd' :
					wysos_suite_email_mail_rcvd($form_state['values']['company_mail_id']);
					// Update Company Mail
					
					db_update('wra_company_mail')
						->fields(array(
							'location' => 'fwd',
							'bin' => strtoupper(substr($rCompany->user_name,0,1)),
						))->condition('company_mail_id',$form_state['values']['company_mail_id'])->execute();
					
					drupal_set_message('Mail for '.$rCompany->company_name.' - Write user <i><b>'.$rCompany->user_name.'</b></i> - Place in <b>FORWARD '.strtoupper(substr($rCompany->user_name,0,1)).'</b>');
					drupal_goto('wysos/suite/mail');
					break;

				case 'scan' :
					// Update Company Mail
					/*
					db_query("UPDATE wra_company_mail SET location='scan', bin='%s' WHERE company_mail_id=%d", 
							strtoupper(substr($rCompany['user_name'],0,1)), $form_state['values']['company_mail_id']);
					*/		
					db_update('wra_company_mail')
						->fields(array(
							'location' => 'scan',
							'bin' => strtoupper(substr($rCompany->user_name,0,1)),
						))->condition('company_mail_id',$form_state['values']['company_mail_id'])->execute();
						
					drupal_set_message('Mail for '.$rCompany->company_name.' - Write user <i><b>'.$rCompany->user_name.'</b></i> - Place in <b>SCAN '.strtoupper(substr($rCompany->user_name,0,1)).'</b>');
					drupal_goto('wysos/suite/mail');
					break;
			}
			drupal_goto('wysos/mail/filing/');
			break;
			
		case 'edit-cancel' :
			drupal_goto('wysos/mail/filing/');
			break;
		
	}
}
function wysos_suite_upload_envelope($unique_id) {
	if(ip_address() != '72.175.205.202') {
		drupal_access_denied();
		return;
	}
//	watchdog('upload_envelope', 'wysos_suite_upload_envelope('.$unique_id.')');
	$ul_dir = '/home/wyregnet/public_html/my/envelopes/';
	$putdata=file_get_contents('php://input'); 

	file_put_contents($ul_dir.arg(3),$putdata);
	
	db_update('wra_company_mail')
		->fields(array(
			'filename' => arg(3),
		))->condition('unique_id' ,substr(arg(3), 0, strpos(arg(3), '.')))->execute();
}
function wysos_suite_suspend($suite_id) {
	// Retrieve wra_suite record
	if(!($rowSuite = db_query("SELECT s.*,c.company_name FROM wra_suites s LEFT JOIN wra_companies c ON s.company_id=c.company_id WHERE s.suite_id=:suite_id", array(':suite_id'=>$suite_id))->fetchObject()))
		return FALSE;
	//if(!($rowSuite = db_fetch_array($r_suite)))
		//return FALSE;
		
	// Retrieve wra_suite_resources records
	$r_resource = db_query("SELECT * FROM wra_suite_resources WHERE suite_id=:suite_id", array(':suite_id'=>$suite_id))->fetchAll();
	if(!($r_resource)){
		return FALSE;
	}	
	$resource = array();
	foreach($r_resource as $rowResource) {
		$resource[$rowResource->resource_type] = array(
			'value' => 	$rowResource->value,
			'data' => unserialize($rowResource->data) );
	}

	/*
	db_query("DELETE FROM fed_wyisdn.voicemail_users WHERE context='default' AND mailbox=%d", $rowSuite['suite_no']);
	db_query("DELETE FROM fed_wyisdn.fax2email WHERE phone_num='%s'", '1'.$resource['fax_no']['value']);
	db_query("DELETE FROM fed_sip.sip_buddies WHERE name='%s' OR username='%s'", $resource['sip_account']['value'], $resource['sip_account']['value']);
	db_query("DELETE FROM fed_wyisdn.extensions WHERE exten='%s'", substr($resource['phone_no']['value'],-7));
	db_query("DELETE FROM fed_wyisdn.extensions WHERE exten='%s'", substr($resource['fax_no']['value'],-7));
	*/
	
	//db_query("UPDATE wra_suites SET status='suspended' WHERE suite_id=%d", $suite_id);
	
	db_update('wra_suites')
			->fields(array(
			'status' => 'suspended',
			))->condition('suite_id',$suite_id)->execute();
	
	require_once('wysos.util.inc');

	$note = 'Suite #'.$rowSuite->suite_no.' for '.$rowSuite->company_name.' suspended.';
	$category = 'suspended';
	wysos_add_user_note($rowSuite->uid,$category, $note);
	drupal_set_message($note);

	drupal_goto('/');
}

function wysos_suite_email_vo_due($suite_id='', $order_id='', $to_email=NULL){

	$result = db_select('wra_suites', 's');
	$result->join('uc_addresses_defaults', 'uad', 'uad.uid = s.uid');
	$result->join('uc_addresses', 'ua', 'ua.aid=uad.aid');
	$result->fields('s');
	$result->fields('ua');
	$result->condition('s.suite_id',$suite_id,'=');
	$rows = $result->execute()->fetchObject();
	//echo '<pre>';
	//print_r($rows);
	//die();
	if(!$rows){
		die('Invalid query');
	}
	if(is_null($to_email)){
			$to_email='kvogt@wyomingregisteredagent.com';
	}
	
	$account = user_load($rows->uid);
	
	$params = array();
	foreach($rows as $key=>$value){
			$params[$key] = $value;	
	}
	
	$result = db_select('profile_field','pf');
	$result->join('profile_value','pv','pf.fid=pv.fid');
	$result->fields('pf', array('name','type'));
	$result->fields('pv', array('value'));
	$result->condition('pv.uid',$account->uid);
	$result = $result->execute()->fetchAll();
	
	foreach($result as $field){

			$params[$field->name] =$field->type.'='.$field->value;
	}
	$params['!order_id'] = $order_id;
	$to_email = $account->mail;
	$message = array(
	  'to' => $to_email,
	  'subject' => t('Virtual Office Payment Due for !street_address #!suite_no', $params),
	  'body' => t("Hello !first_name,\n\nThe monthly payment for your Wyoming virtual office is now due. You can pay the $!price fee online and set up a recurring charge to a credit card by going to:\n\nhttps://my.wyomingregisteredagent.com/pay_vo_invoice/!suite_id/!order_id\n\nAlternatively, please send a check or money order for $!price to:\n\nWyomingRegisteredAgent.com, Inc.\n1621 Central Ave.\nDept. VO!order_id\nCheyenne, WY 82001\n\n\nThank you.", $params),
	  'headers' => array('From' => 'info@wyomingregisteredagent.com'),
	);
	//drupal_mail_send($message); 
	
	drupal_set_message('Virtual office due sent to '.$account->mail.' for '.$rows->street_address.' #'.$rows->suite_no.'.');
	require_once('wysos.util.inc');
	wysos_add_user_note($rows->uid, 'System', 'Virtual office due sent to '.$account->mail.' for '.$rows->street_address.' #'.$rows->suite_no.'.');

	drupal_goto('/');
}
function wysos_suite(){
	drupal_set_title('Mail Suites');
	$headers = array(
			array('data' => t('Suite'),
				'fields' => 'suite_no',
				'sort' => 'ASC' ),
			array('data' => t('Street Address'),
				'fields' => 'street_address'),
			array('data' => t('Company Name'),
				'fields' => 'company_name'),
			array('data' => t('User'),
				'fields' => 'user_name'),
			array('data' => t('Term'),
				'fields' => 'billing_period'),
			array('data' => t('Status'),
				'fields' => 'active'),
				'options',
		);
		
		
		$query = db_query("SELECT s.suite_id, s.company_id, LEFT(IFNULL(c.company_name, s.company_name),30) AS company_name, s.street_address, s.suite_no, s.uid, u.name AS user_name,
			s.billing_period, s.status
	FROM wra_suites s
	LEFT JOIN wra_companies c ON s.company_id=c.company_id
	LEFT JOIN users u ON s.uid=u.uid
	WHERE s.hidden!=1 AND s.billing_period != 'internal'")->fetchAll();
	/*	
	$query = db_select('wra_suites', 's');
	$query->leftJoin('wra_companies','c','s.company_id=c.company_id');
	$query->leftJoin('users','u','s.uid=u.uid');
	$query->fields('s',array('suite_id','company_id','street_address','suite_no','uid'));
	$query->addfield('u','name','user_name');
	$query->condition('s.hidden','1','<>');
	$query->condition('s.billing_period','internal','<>');
	$query = $query->execute()->fetchAll();
	*/
	
	$rows = array();
	foreach ($query as $suite) {
		$options = 	l('Details','wysos/suite/browse/'.$suite->suite_no);
		$options .= ' '.l(t('Edit'),'wysos/suite/edit/'.$suite->suite_no);

    	$rows[] = array(l($suite->suite_no,'wysos/suite/browse/'.$suite->suite_no),
		 	$suite->street_address,
			$suite->company_name,
			l($suite->user_name,'wysos/user/'.$suite->uid),
			$suite->billing_period,
		 	$suite->status,
			$options
			);
	}
	  if (!$rows) {
		$rows[] = array(array('data' => t('No client accounts created yet.'), 'colspan' => 3));
	  }
    $per_page = 25;
	$current_page = pager_default_initialize(count($rows), $per_page);
	$chunks = array_chunk($rows, $per_page, TRUE);
	$output = theme('table', array('header' => $headers, 'rows' => $chunks[$current_page]));
	// $output .= theme('table', $headers, $rows);
	//$output .= theme('pager', NULL, $limit, 0);
	$output .= theme('pager', array('quantity',count($rows)));
	return $output;		
}
function wysos_suite_instructions($suite_id = NULL){
	$row = db_query("SELECT s.*, c.company_name, u.mail as email, r1.value as sip_user, r2.value as sip_password, r3.value as did, r4.value as fax FROM wra_suites s
		LEFT JOIN wra_companies c ON s.company_id = c.company_id
		INNER JOIN users u ON s.uid = u.uid
		LEFT JOIN wra_suite_resources r1 ON s.suite_id = r1.suite_id AND r1.resource_type = 'sip_user'
		LEFT JOIN wra_suite_resources r2 ON s.suite_id = r2.suite_id AND r2.resource_type = 'sip_password'
		LEFT JOIN wra_suite_resources r3 ON s.suite_id = r3.suite_id AND r3.resource_type = 'did'
		LEFT JOIN wra_suite_resources r4 ON s.suite_id = r4.suite_id AND r4.resource_type = 'fax'
	WHERE s.suite_id =:suite_id",array(':suite_id'=>$suite_id))->fetchObject();
	
	//echo '<pre>';
	//print_r($row);
	//die();
	if($row){
		
	$GLOBALS['company_name'] = $row->company_name;
	$GLOBALS['suite_no'] = $row->suite_no;
	$GLOBALS['phone_no'] = '+1 (' . SUBSTR($row->did,0,3).') '.SUBSTR($row->did,3,3).'-'.SUBSTR($row->did,6,4);
	$GLOBALS['fax_no'] = '+1 (' . SUBSTR($row->fax,0,3).') '.SUBSTR($row->fax,3,3).'-'.SUBSTR($row->fax,6,4);
	$GLOBALS['email'] = $row->email;
	$GLOBALS['sip_user'] = isset($row->sip_user)?$row->sip_user:'';
	$GLOBALS['sip_password'] = isset($row->sip_password)?$row->sip_password:'';

	// Open the OpenOffice Template
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');

	$TBS = new clsTinyButStrong;
	$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

	$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/VirtualOfficeInstructions.odt');
	$TBS->Show(TBS_OUTPUT, 'VOInstruct-'.$row->company_name.'.odt');
	//	$TBS->Show();
}
}

/*
 * Thank you
 */
function wysos_thankyou() {
	$output = 'Thank you for the update.';
	return $output;
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












