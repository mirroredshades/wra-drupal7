<?php

function wysos_virtualoffice_email_for_signup($suite_id, $email=NULL){
	drupal_set_message('wysos_virtualoffice_email_for_signup('.$suite_id.')');
	global $user;
	$result = db_query("SELECT s.suite_id, s.suite_no, s.brand,c.company_name, s.uid, u.name AS user_name, s.price, s.status, s.pay_by, s.service_level, s.billing_period, a.first_name, a.last_name, u.mail
	FROM wra_suites s 
	INNER JOIN users u ON s.uid = u.uid
	INNER JOIN uc_addresses_defaults ad ON ad.uid = u.uid
	INNER JOIN uc_addresses a ON a.aid = ad.aid
	LEFT JOIN wra_companies c ON s.company_id = c.company_id
	WHERE s.suite_id = ".$suite_id)->fetchObject();
	
	$url = wysos_virtualoffice_signup_url($suite_id);
	$services = '';
	if($result){
	switch($result->service_level){
		case 'bronze' :
			$services = 'mail forwarding service';
			break;
		
		case 'silver' : 
			$services = 'mail forwarding, Wyoming phone number with unlimited U.S. and Canada long distance, and Wyoming fax number services';
			break;
			
		case 'gold' :
			$services = 'mail forwarding, Wyoming phone number with live receptionist, and Wyoming fax number services';
			break;
	}
	
	switch ($result->brand){
		case 'WyomingRegisteredAgent.com' :
			$from = 'Wyoming Registered Agent <info@wyomingregisteredagent.com>';
			$signature = "WyomingRegisteredAgent.com, Inc.\r\n307-637-5151";
			break;
			
		case '17th & Central Executive Suites' :
			$from = '17th & Central Executive Suites <info@17thandcentral.com>';
			$signature = "17th & Central Executive Suites\r\n307-432-4099";
			break;
	}
	$subject = "Payment Failed for ".$result->company_name." Virtual Office";
	if(empty($email)) 
		$to = $result->mail;
	else
		$to = $email;
	//$to='kvogt@wyomingregisteredagent.com';	
	$headers   = array();
	$headers[] = "MIME-Version: 1.0";
	$headers[] = "Content-type: text/plain; charset=iso-8859-1";
	$headers[] = "From: ".$from;
	$headers[] = "Subject: ".$subject;
	$headers[] = "X-Mailer: PHP/".phpversion();
	$url2=isset($url)?$url:'';
	$body  = "Dear ".$result->first_name.",\r\n\r\n";
	$body .= "Recently, we were unable to process a payment of ".uc_currency_format($result->price)." for your Wyoming virtual office for ".$result->company_name.".\r\n\r\n";
	$body .= "To continue with the ".$services.", please update your payment information now by going to the link below:\r\n\r\n";
	$body .= $url2."\r\n\r\n";
	$body .= "You can login to your account with user name ".$result->user_name."\r\n";
	$body .= "To have your password emailed to you, please goto https://my.wyomingregisteredagent.com/user/password\r\n\r\n";
	$body .= "Thank you,\r\n".$signature;
	
	mail($to, $subject, $body, implode("\r\n", $headers));
	
	$message = "Request for signup credit card for suite ".$result->suite_no." (".$result->company_name.") emailed to ".$to;
	$result = db_query("INSERT INTO wra_user_notes (uid, created_uid, category, notes, created) 
		VALUES(".$result->uid.",".$user->uid.",'virtual_office','".mysql_real_escape_string($message)."',UNIX_TIMESTAMP())");
			
	drupal_set_message($message);
	db_query("UPDATE wra_suites SET status='suspended' WHERE suite_id=".$suite_id);
	//	drupal_goto(referer_uri());
}
}

function  wysos_virtualoffice_signup_url($suite_id){
	$result = db_query("SELECT s.suite_id, s.suite_no, s.brand, c.company_name, s.uid, u.name AS user_name, s.price, s.status, s.pay_by, s.chargify_subscription_id, s.service_level, s.billing_period, a.first_name, a.last_name, u.mail, s.street_address
	FROM wra_suites s
	INNER JOIN users u ON s.uid = u.uid
	INNER JOIN uc_addresses_defaults ad ON ad.uid = u.uid
	INNER JOIN uc_addresses a ON a.aid= ad.aid
	LEFT JOIN wra_companies c ON s.company_id = c.company_id
	WHERE s.suite_id=". $suite_id)->fetchObject();
	
	if($result){
	
	switch ($result->street_address){
		case '109 E. 17th St.' :
			$street_option = 92;
			break;
			
		case '1623 Central Ave.' :
			$street_option = 93;
			break;
	}
	switch($result->service_level) {
		case 'bronze' :
			$services = 'mail forwarding service';
			switch($result->billing_period) {
				case 'month'	:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1851_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					else if($result->brand == '17th & Central Executive Suites')
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1851_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					break;					
					
				case 'year'		:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1852_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					else if($result->brand == '17th & Central Executive Suites')
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1852_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					break;
			}
			break;
			
		case 'silver' :
			$services = 'mail forwarding, Wyoming phone number with unlimited U.S. and Canada long distance, and Wyoming fax number services';
			switch($result->billing_period) {
				case 'month'	:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1853_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					else if($result->brand == '17th & Central Executive Suites')
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1853_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					break;
					
				case 'year'		:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1854_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					else if($result->brand == '17th & Central Executive Suites')
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1854_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					break;
			}
			break;
			
		case 'gold'	:
			$services = 'mail forwarding, Wyoming phone number with live receptionist, and Wyoming fax number services';
			switch($result->billing_period) {
				case 'month'	:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1855_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					else if($result->brand == '17th & Central Executive Suites')
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1855_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					break;
					
				case 'year'		:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1856_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					else if($result->brand == '17th & Central Executive Suites')
						$url = "https://my.wyomingregisteredagent.com/cart/add/p1856_a7o".urlencode($result->company_name)."_a47o".$street_option."_a41o".urlencode($result->suite_no)."_m0?destination=cart/checkout";
					break;
			}
			break;
	}
	return $url;
	
}
}

function wysos_virtual_office_monthly_list_form($form,&$form_state)
{
	$header = array( 'Suite', 'Company', 'User', 'Balance', 'Price', 'Last Invoice',  'Status', 'Pay By', 'Options'  );

	$result = db_query("SELECT DISTINCT s.anniv
		FROM wra_suites s 
		INNER JOIN users u ON s.uid=u.uid
		LEFT JOIN wra_companies c ON s.company_id=c.company_id
		WHERE s.billing_period='month' AND s.suite_id NOT IN (SELECT suite_id FROM wra_office_suite)
		ORDER BY s.anniv")->fetchAll();
//$result = db_query("SELECT 1");		

	if (!$result) {
		die('Invalid query');
	}  

	foreach ( $result as $row ) {
		if( is_null($row->anniv)) {
			$form['day_NULL'] = array(
				'#title' => '(NULL)',
				'#type' => 'fieldset',
				'#weight' => -99,
				'#collapsible' => TRUE, 
				'#collapsed' => TRUE
			);
			
			$day_row = wysos_virtual_office_list_anniv('month', NULL);
			
			//$form['day_NULL']['list'] = array('#type' => 'markup', '#value' => theme('table', $header, $day_row));
			$form['day_NULL']['list'] = array('#markup'=>theme('table', array('header'=>$header,'rows' => $day_row)));
			//print_r($form['day_NULL']['list']);
			//die();
		} else {
			$form['day_'.$row->anniv] = array(
				'#title' => $row->anniv,
				'#type' => 'fieldset',
				'#weight' => $row->anniv,
				'#collapsible' => TRUE, 
				'#collapsed' => TRUE
			);
			
			$day_row = wysos_virtual_office_list_anniv('month', $row->anniv);
			$form['day_'.$row->anniv]['list'] = array('#markup'=>theme('table', array('header'=>$header,'rows' => $day_row)));
			//$form['day_'.$row->anniv]['list'] = array('#type' => 'markup', '#value' => theme('table', $header, $day_row));
		}

	
	}
		
	return $form;
	
}
function wysos_virtual_office_list_anniv($period, $anniv) {
	$chargify_shared_key = '6GTYVBai0QA4EZ6JkCIb';
	$header = array( 'Suite', 'Company', 'User', 'Balance', 'Price', 'Last Invoice',  'Status', 'Pay By', 'Options'  );
	switch($period) {
		case 'month' :
			$products = "'WYVO1M','WYVODM'";
			break;
			
		case 'year' :
			$products = "'WYVO1Y,'WYVODY'";
			break;
	}
	if( is_null($anniv)) {
		$anniv_sql = ' AND s.anniv IS NULL ';
	} else {
		$anniv_sql = ' AND s.anniv='.$anniv.' ';
	}
	$result = db_query("SELECT s.suite_id, s.suite_no, s.brand, c.company_name, s.uid, u.name AS user_name, s.price, s.status, s.pay_by, s.chargify_subscription_id, s.service_level, 
		s.billing_period, a.first_name, a.last_name, u.mail,
		(SELECT o.order_id FROM uc_orders o INNER JOIN uc_order_products p ON  o.order_id=p.order_id WHERE o.uid=s.uid AND p.model IN(".$products.") ORDER BY o.order_id DESC LIMIT 1) AS last_order,
		(SELECT SUM(order_total) FROM uc_orders WHERE uid=u.uid AND order_status IN('completed','payment_received')) -
IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id IN(SELECT order_id FROM uc_orders WHERE uid=u.uid AND order_status IN('completed','payment_received'))),0) AS balance_due
		FROM wra_suites s 
		INNER JOIN users u ON s.uid=u.uid
		INNER JOIN uc_addresses_defaults ad ON ad.uid =u.uid
		INNER JOIN uc_addresses a ON a.aid=ad.aid
		LEFT JOIN wra_companies c ON s.company_id=c.company_id
		WHERE s.billing_period='".$period."' AND s.suite_id NOT IN (SELECT suite_id FROM wra_office_suite) ".$anniv_sql." 
		ORDER BY s.suite_no")->fetchAll();

	$suite = array();
	
	
	
	foreach( $result as $row) {
	
		$options = '<font size=-3>';

		$options .= l('Email_Signup', 'chargify/email_signup/'.$row->suite_id).' ';
		if($row->pay_by == 'chargify') {
			if(empty($row->chargify_subscription_id)) {
				$options .= l('Create Subscription', 'chargify/subscription/create/'.$row->suite_id);
			} else {
				$message = 'update_payment--'.$row->chargify_subscription_id.'--'.$chargify_shared_key;
				$url = 'https://wyoming-registered-agent.chargify.com/update_payment/'.$row->chargify_subscription_id.'/'.sha1($message);
				$options .= l('Subscription', 'chargify/subscription/view/'.$row->chargify_subscription_id ).' ';	
				$options .= l('New CC', $url).' ';
			}
		} else {
//drupal_set_message('svc-bill-brand: '.$row['service_level'].'-'.$row['billing_period'].'-'.$row['brand']);
			include_once('wysos.chargify.inc');
			
			$url = wysos_chargify_signup_url($row->suite_id);
			if($url) {
				$options .= l('Signup URL', $url).' ';
			}
		}
	
		$options .= '</font>';
		$suite[] = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => l($row->suite_no, 'wysos/suite/', array('attributes' => array('target' => '_suite')))),
				array('data' => $row->company_name),
				array('data' => l($row->user_name,'wysos/user/'.$row->uid)),
				array('data' => uc_currency_format($row->balance_due)),
				array('data' => uc_currency_format($row->price)),
				array('data' => l($row->last_order, 'admin/store/orders/'.$row->last_order) ),
				array('data' => $row->status),
				array('data' => $row->pay_by),
				array('data' => $options)));
	}	
	return $suite;
}
function wysos_virtual_office_admin() {
	$form = array();
	$form['dl-chargify'] = array(
		'#type' => 'submit',
		'#value' => 'Download',
		'#description' => 'Download Chargify Subscriptions'
		);
		
	return $form;
		
}
function wysos_virtual_office_admin_submit($form, &$form_state) {
	drupal_set_message('clicked: '.$form_state['clicked_button']['#id']);
	switch($form_state['clicked_button']['#id'])
	{
		case 'edit-dl-chargify'	:
			break;
			
	}
		
}
function wysos_virtual_office_instructions($suite_id, $download_now=true)
{
//drupal_set_message('wysos_virtual_office_instructions('.$suite_id.')');
	$row = db_query("SELECT s.*, u.mail as email, u.name as user_name, r1.value as sip_user, r2.value as sip_password, r3.value as did, r4.value as fax,
		r5.value as vmail2email, r6.value as fax2email 
		FROM wra_suites s
		LEFT JOIN wra_companies c ON s.company_id = c.company_id
		INNER JOIN users u ON s.uid = u.uid
		LEFT JOIN wra_suite_resources r1 ON s.suite_id = r1.suite_id AND r1.resource_type = 'sip_user'
		LEFT JOIN wra_suite_resources r2 ON s.suite_id = r2.suite_id AND r2.resource_type = 'sip_password'
		LEFT JOIN wra_suite_resources r3 ON s.suite_id = r3.suite_id AND r3.resource_type = 'did'
		LEFT JOIN wra_suite_resources r4 ON s.suite_id = r4.suite_id AND r4.resource_type = 'fax'
		LEFT JOIN wra_suite_resources r5 ON s.suite_id = r5.suite_id AND r5.resource_type = 'vmail2email'
		LEFT JOIN wra_suite_resources r6 ON s.suite_id = r6.suite_id AND r6.resource_type = 'fax2email'
	WHERE s.suite_id =:suite_id", array(':suite_id'=>$suite_id))->fetchObject();
	//$row = db_fetch_array($result);
	$download_url = 'wysos/download/';
	$download_path = $_SERVER['DOCUMENT_ROOT'].'/wra_dl/';
	
	switch($row->brand) {
		case 'WyomingRegisteredAgent.com'	:
			$brand = 'WRA';
			break;
			
		case '17th & Central Executive Suites' :
			$brand = 'SAC';
			break;
	}
//	drupal_set_message('company_name: '.$row['company_name']);
	
	switch($row->service_level) {
		case 'bronze' :
			$slvl = 'Bronze';
			// Set Globals
			$GLOBALS['company_name'] = $row->company_name;
			$GLOBALS['street_address'] = $row->street_address;
			$GLOBALS['suite_no'] = $row->suite_no;
			$GLOBALS['user_name'] = $row->user_name;
	
			// Open the OpenOffice Template
			include_once('tbs_class.php');
			include_once('tbs_plugin_opentbs.php');
		
			$TBS = new clsTinyButStrong;
			$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
		
			$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/VO-'.$brand.'-'.$slvl.'.odt');
//			$TBS->Show(OPENTBS_FILE, $download_path.'VOInstruct-'.$row->company_name.'.odt');
			if($download_now) {
				$TBS->Show(TBS_OUTPUT, 'VOInstruct-'.$row->company_name.'.odt');
				break;
			} else {
				$download_file = 'VOInstruct-'.$row->company_name.'.odt';
				$file_path = $download_path.'VOInstruct-'.$row->company_name.'.odt';
				$TBS->Show(OPENTBS_FILE, $file_path);
				include_once('wysos.download.inc');
				$fid = wysos_download_add_file($row->company_name." Virtual Office Instructions", $download_file);
				return $fid;
				break;
			}
			break;
			
		case 'silver' :
		case 'gold' :
			if($row->service_level == 'silver') {
				$slvl = 'Silver';
			} elseif($row->service_level == 'gold') {
				$slvl = 'Gold';
			}
			// Set Globals
			$GLOBALS['company_name'] = $row->company_name;
			$GLOBALS['street_address'] = $row->street_address;
			$GLOBALS['suite_no'] = $row->suite_no;
			$GLOBALS['phone_no'] = '+1 (' . SUBSTR($row->did,0,3).') '.SUBSTR($row->did,3,3).'-'.SUBSTR($row->did,6,4);
			$GLOBALS['fax_no'] = '+1 (' . SUBSTR($row->fax,0,3).') '.SUBSTR($row->fax,3,3).'-'.SUBSTR($row->fax,6,4);
			$GLOBALS['vmail2email'] =$row->vmail2email;
			$GLOBALS['fax2email'] = $row->fax2email;
			$GLOBALS['extension'] = $row->suite_no;
			$GLOBALS['passcode'] = $row->suite_no;
			$GLOBALS['sip_user'] = $row->sip_user;
			$GLOBALS['sip_password'] = $row->sip_password;
			$GLOBALS['user_name'] = $row->user_name;
	
			// Open the OpenOffice Template
			include_once('tbs_class.php');
			include_once('tbs_plugin_opentbs.php');
		
			$TBS = new clsTinyButStrong;
			$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
		
			$txtFile = 	filesystem_base_path().'/sites/all/modules/wysos/VO-'.$brand.'-'.$slvl.'.odt';
			$TBS->LoadTemplate($txtFile);
//			$TBS->Show(OPENTBS_FILE, $download_path.'VOInstruct-'.$form_state['storage']['values'][2]['company_name'].'.odt');
			if($download_now) {
				$TBS->Show(TBS_OUTPUT, 'VOInstruct-'.$row->company_name.'.odt');
				break;
			} else {
				$download_file = 'VOInstruct-'.$row->company_name.'.odt';
				$file_path = $download_path.'VOInstruct-'.$row->company_name.'.odt';
				$TBS->Show(OPENTBS_FILE, $file_path);
				include_once('wysos.download.inc');
				$fid = wysos_download_add_file($row->company_name." Virtual Office Instructions", $download_file);
				return $fid;
				break;
			}
			break;
	}
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
