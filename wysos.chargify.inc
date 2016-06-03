<?php
function wysos_chargify_create_subscription_form($form, &$form_state,$suite_id){
	$form_state['storage']['suite_id'] = $suite_id;
	$result = db_query('SELECT s.*, u.mail, u.name AS user_name, u.mail, a.first_name, a.last_name 
	FROM wra_suites s 
	INNER JOIN users u ON s.uid = u.uid
	INNER JOIN uc_addresses_defaults d ON s.uid = d.uid
	INNER JOIN uc_addresses a ON d.aid = a.aid
	WHERE suite_id ='.$suite_id)->fetchObject();
	
	$form['name'] = array(
		'#title' => t('User'),
		'#type' => 'textfield',
		'#maxlength' => 60,
		'#value' => $result->user_name,
		'#disabled' => TRUE,
	);
	$form['brand'] = array(
		'#title' => 'select',
		'#type' => t('Brand'),
		'#options' => array( 'WyomingRegisteredAgent.com' => 'WyomingRegisteredAgent.com', '17th & Central Executive Suites'=>'17th & Central Executive Suites'),
		'#default_value' => $result->brand,
	);
	$default_service_level ='';
	switch($result->service_level){
		case 'bronze' :
			$default_service_level = 'vobrm';
			break;
		case 'silver' :
			$default_service_level = 'vosim';
			break;
		case 'gold' :
			$default_service_level = 'vogom';
			break;
	}
	$form['service_level'] = array(
		'#type' => 'select',
		'#title' => t('Package'),
		'#options' => array( 'vobrm' => 'Bronze ($29/month)', 'vosim' => 'Silver ($49/month)', 'vogom' => 'Gold ($79/month)'),
		'#default_value' => isset($_POST['service_level']) ? $_POST['service_level'] : $default_service_level,
	);
	$newdate = strtotime ('+1 month');
	
	$form['next_billing_date'] = array(
		'#type' => 'date',
		'#title' => t('Next Billing Date'),
		'#default_value' => array(
			'month' => format_date($newdate, 'custom', 'n'),
			'day' => format_date($newdate, 'custom', 'j'),
			'year' => format_date($newdate, 'custom', 'Y'),
		),
	);
	$form['payment'] = array(
		'#title' => t('Payment'),
		'#prefix' => '<div id="payment">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
	);
	$form['payment']['cc_number'] = array(
		'#type' => 'textfield',
		'#title' => t('Card number'),
		'#default_value' => isset($_POST['cc_number']) ? $_POST['cc_number'] : '',
		'#attributes' => array('autocomplete' => 'off'),
		'#maxlength' => 19,
	);
	if(isset($result->cc_exp)){
		$exp_month = substr($result->cc_exp,0,2);
		$exp_year = substr($result->cc_exp,3);
		$form['payment']['cc_exp_field'] = array(
			'#type' => 'item',
			'#title' => 'Web Order Card Expiration',
			'#value' => $result->cc_exp		
		);
	}
	//$form['payment']['cc_exp_month'] = uc_select_month(t('Expiration Month'), isset($_POST['cc_exp_month']) ? $_POST['cc_exp_month']:$exp_month);
	//$form['payment']['cc_exp_year'] = us_select_year(t('Expiration Year'), isset($_POST['cc_exp_year']) ? $_POST['cc_exp_year']:$exp_year);
	 $form['payment']['cc_exp_month'] = array(
        '#title' => t('Month'),
        '#type' => 'select',
        '#options' => array(1=>'Jan (1)',2=>'Feb (2)',3=>'Mar (3)',4=>'Apr (4)',5=>'May (5)',6=>'Jun (6)',7=>'Jul (7)',8=>'Aug (8)',9=>'Sep (9)',
                    10=>'Oct (10)',11=>'Nov (11)',12=>'Dec (12)'),
        '#default_value' => date('n'),
        '#required' => TRUE,
    );
    $year = date('Y');
    $form['payment']['cc_exp_year'] = array(
        '#title' => t('Year'),
        '#type' => 'select',
        '#options' => array($year-1=>$year-1,$year=>$year,$year+1=>$year+1),
        '#default_value' => $year,
        '#required' => TRUE,
    );
	$form['payment']['billing_first_name'] = array(
		'#title' => t('First Name'),
		'#type' => 'textfield',
		'#size' => '60',
		'#default_value' => $result->first_name,
	);
	$form['payment']['billing_last_name'] = array(
		'#title' => t('Last Name'),
		'#type' => 'textfield',
		'#size' => '60',
		'#default_value' => $result->last_name,
	);
	$form['payment']['billing_street1'] = array(
		'#title' => t('Address'),
		'#type' => 'textfield',
		'#default_value' => isset($result->cc_addr)?$result->cc_addr:'',
	);
	$form['payment']['billing_street2'] = array(
		'#type' => 'textfield',
		'#default_value' => '',
	);
	$form['payment']['billing_city'] = array(
		'#title' => t('City'),
		'#type' => 'textfield',
		'#default_value' => '',
	
	);
	
	if (isset($_POST['billing_country'])) {
    	$country_id = intval($_POST['billing_country']);
  	} else {
    	$country_id = NULL;
  	}
	
	//$form['payment']['billing_country'] = uc_country_select(uc_get_field_name('country'));
	$countries = db_query("SELECT country_id,country_name FROM uc_countries")->fetchAll();
	$country_list = array(); 
	foreach($countries as $country){
		$country_list[$country->country_id] = $country->country_name;
	}
	$form['payment']['billing_country'] = array(
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
	$form['payment']['billing_zone'] = array(
		'#type' => 'select',
		'#options' => $zone_list,
		'#title' => t('State/Province'),
		'#id' => 'tzone', 
	);
	$form['payment']['zone_load'] = array(
		'#type' => 'item',
	);
	$form['payment']['billing_postal_code'] = array(
		'#title' => t('Zip/Postal Code'),
        '#type' => 'textfield',
        '#size' => '12',
		'#maxlength' => '40',
        '#default_value' => isset($row['cc_zip'])?$row['cc_zip']:'',
	);
	$form['create-subscription'] = array(
		'#type' => 'submit',
		'#id' => 'create-subscription',
		'#value' => 'Create Subscription'
		);
	$form['create-customer'] = array(
		'#type' => 'submit',
		'#id' => 'create-customer',
		'#value' => 'Create Customer & Email for Card'
		);
		
	return $form;
	
}
function wysos_chargify_create_subscription_form_validate($form, &$form_state) {
}
function wysos_chargify_create_subscription_form_submit($form, &$form_state) {

	$result = db_query("SELECT a.*, z.zone_code AS state, c.country_iso_code_2 AS country 
		FROM uc_addresses a 
		INNER JOIN uc_addresses_defaults d ON a.uid=d.uid 
		LEFT JOIN uc_zones z ON a.zone=z.zone_id 
		LEFT JOIN uc_countries c ON a.country = c.country_id WHERE a.uid=".$form_state['storage']['suite_id'])->fetchObject();
		//echo '<pre>';
		//print_r($result);
		//die();
	//$row = db_fetch_array($result);
	$email = isset($form_state['storage']['suite']['mail'])?$$form_state['storage']['suite']['mail']:'';
	$billing_state = db_query("SELECT zone_code FROM uc_zones WHERE zone_id=".$form_state['values']['billing_zone']);
//drupal_set_message('row: '.print_r($row, true));			
	switch($form_state['clicked_button']['#id']) {
		case 'create-subscription' :
			$array_subscription = array(
				'product_handle' => $form_state['values']['service_level'],
				'customer_attributes' => array(
					'first_name' => $result->first_name,
					'last_name' => $result->last_name,
					'email' => $email,
					'organization' => $result->company,
					'reference' => $form_state['storage']['suite_id'],
					'address' => $result->street1,
					'address_2' => $result->street2,
					'city' => $result->city,
					'state' => $result->state,
					'zip' => $result->postal_code,
					'country' => $result->country),
				'credit_card_attributes' => array(
					'first_name' => $form_state['values']['billing_first_name'],
					'last_name' => $form_state['values']['billing_last_name'],
//						'billing_address' => $form_state['values']['billing_address'],
					'billing_address' => empty($form_state['values']['billing_street2']) ? $form_state['values']['billing_street1'] : $form_state['values']['billing_street1'] .
						'\n'.$form_state['values']['billing_street2'],
					'billing_city' => $form_state['values']['billing_city'],
					'billing_state' => $billing_state,
					'billing_zip' => $form_state['values']['billing_postal_code'],
					'billing_country' => $form['payment']['billing_country']['#options'][$form_state['values']['billing_country']],
					'full_number' => $form_state['values']['cc_number'],
					'expiration_month' => $form_state['values']['cc_exp_month'],
					'expiration_year' => $form_state['values']['cc_exp_year']), 
				'next_billing_at' => $form_state['values']['next_billing_date']['year'].'-'.$form_state['values']['next_billing_date']['month'].'-'.$form_state['values']['next_billing_date']['day']
				);
	drupal_set_message('array_subscription: <pre>'.print_r($array_subscription, true).'</pre>');			
				
			// creating object of SimpleXMLElement
			$xml_subscription = new SimpleXMLElement("<?xml version=\"1.0\"?><subscription></subscription>");

			// function call to convert array to xml
			array_to_xml($array_subscription,$xml_subscription);
			$xml_subscription->asXML("/home/wyregent/xml_out.txt");
			file_put_contents("/home/wyregnet/xml_out1.txt", $xml_subscription->asXML());
			
			//saving generated xml file
			$subscription = chargify_api_subscription_create($xml_subscription->asXML());
			drupal_set_message('subscription: '.print_r($subscription, true));
/*			if(isset($subscription->error)) {
				drupal_set_message('subscription error: '.print_r($subscription, true));
			} else {
				drupal_set_message('new subscription: '.print_r($subscription, true));
				// Save the Subscription ID */

				$result = db_query("UPDATE wra_suites SET chargify_subscription_id=".$subscription->getId().", anniv=".$form_state['values']['next_billing_date']['day']." WHERE suite_id=".$form_state['storage']['suite_id']);
//			}
			break;
			
		case 'create-customer' :
			$array_customer = array(
				'first_name' =>$result->first_name,
				'last_name' => $result->last_name,
				'email' => $form_state['storage']['suite']['mail'],
				'organization' => $result->company,
				'reference' => $form_state['storage']['suite_id'],
				);
drupal_set_message('array_customer: <pre>'.print_r($array_customer, true).'</pre>');			
				
			// creating object of SimpleXMLElement
			$xml_customer = new SimpleXMLElement("<?xml version=\"1.0\"?><customer></customer>");

			// function call to convert array to xml
			array_to_xml($array_customer,$xml_customer);
			file_put_contents("/home/wyregnet/xml_out2.txt", $xml_customer->asXML());
			
			//saving generated xml file
			$customer = chargify_api_customer_create($xml_customer->asXML());
			if(isset($customer->error)) {
				drupal_set_message('customer error: '.print_r($customer, true));
			} else {
				drupal_set_message('new customer: '.print_r($customer, true));
				// Save the Customer
				if(db_result(db_query("SELECT COUNT(*) FROM chargify_customers WHERE customer_id=".$customer->getID()." AND uid=".$form_state['storage']['suite']['uid'])) > 0) {
					// Change Existing
					
				} else {
					// Create New
					$result = db_query("INSERT chargify_customers (customer_id, uid) VALUES (".$customer->getID().",".$form_state['storage']['suite']['uid'].")");
//					chargify_user_save($customer);
				}
			}

	}

}
function array_to_xml($student_info, &$xml_student_info) {
    foreach($student_info as $key => $value) {
        if(is_array($value)) {
            $key = is_numeric($key) ? "item$key" : $key;
            $subnode = $xml_student_info->addChild("$key");
            array_to_xml($value, $subnode);
        }
        else {
            $key = is_numeric($key) ? "item$key" : $key;
            //$xml_student_info->addChild("$key","$value");
        }
    }
}

function wysos_chargify_email_for_signup($suite_id, $email=NULL) {
	global $user;
	$result = db_query("SELECT s.suite_id, s.suite_no, s.brand, c.company_name, s.uid, u.name AS user_name, s.price, s.status, s.pay_by, s.chargify_subscription_id, s.service_level, 
		s.billing_period, a.first_name, a.last_name, u.mail 
		FROM wra_suites s 
		INNER JOIN users u ON s.uid=u.uid
		INNER JOIN uc_addresses_defaults ad ON ad.uid =u.uid
		INNER JOIN uc_addresses a ON a.aid=ad.aid
		LEFT JOIN wra_companies c ON s.company_id=c.company_id
		WHERE s.suite_id=".$suite_id)->fetchObject();
	//$row = db_fetch_array($result);
	
	$url = wysos_chargify_signup_url($suite_id);
	$services = '';
	switch($result->service_level) {
		case 'bronze' :
			$services = 'mail forwarding service';
			break;
			
		case 'silver' :
			$services = 'mail forwarding, Wyoming phone number with unlimited U.S. and Canada long distance, and Wyoming fax number services';
			break;
			
		case 'gold'	:
			$services = 'mail forwarding, Wyoming phone number with live receptionist, and Wyoming fax number services';
			break;
	}
	switch($result->brand) {
		case 'WyomingRegisteredAgent.com' 	:
			$from = 'Wyoming Registered Agent <info@wyomingregisteredagent.com>';
			$signature = "WyomingRegisteredAgent.com, Inc.\r\n307-637-5151";
			break;
			
		case '17th & Central Executive Suites'	:
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

	$body  = "Dear ".$result->first_name.",\r\n\r\n";
	$body .= "Recently, we were unable to process a payment of ".uc_currency_format($result->price)." for your Wyoming virtual office for ".$result->company_name.".\r\n\r\n";
	$body .= "To continue with the ".$services.", please update your payment information now by going to the link below:\r\n\r\n";
	$body .= $url."\r\n\r\n";
	$body .= "Thank you,\r\n".$signature;
	
	mail($to, $subject, $body, implode("\r\n", $headers));
	
	$message = "Request for Chargify signup credit card for suite ".$result->suite_no." (".$result->company_name.") emailed to ".$to;
	$result = db_query("INSERT INTO wra_user_notes (uid, created_uid, category, notes, created) 
		VALUES(".$result->uid.",".$user->uid.",'virtual_office','".mysql_real_escape_string($message)."',UNIX_TIMESTAMP())");
			
	drupal_set_message($message);
	
//	drupal_goto(referer_uri());
}


function wysos_chargify_signup_url($suite_id) {
	$result = db_query("SELECT s.suite_id, s.suite_no, s.brand, c.company_name, s.uid, u.name AS user_name, s.price, s.status, s.pay_by, s.chargify_subscription_id, s.service_level, 
		s.billing_period, a.first_name, a.last_name, u.mail 
		FROM wra_suites s 
		INNER JOIN users u ON s.uid=u.uid
		INNER JOIN uc_addresses_defaults ad ON ad.uid =u.uid
		INNER JOIN uc_addresses a ON a.aid=ad.aid
		LEFT JOIN wra_companies c ON s.company_id=c.company_id
		WHERE s.suite_id=".$suite_id)->fetchObject();
	//$row = db_fetch_array($result);
	
	switch($result->service_level) {
		case 'bronze' :
			$services = 'mail forwarding service';
			switch($result->billing_period) {
				case 'month'	:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$base_url = 'https://wyoming-registered-agent.chargify.com/h/3313013/subscriptions/new';
					else if($result->brand == '17th & Central Executive Suites')
						$base_url = 'https://17th-central-executive-suites.chargify.com/h/3313000/subscriptions/new';
					break;
					
				case 'year'		:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$base_url = 'https://wyoming-registered-agent.chargify.com/h/3313014/subscriptions/new';
					else if($result->brand == '17th & Central Executive Suites')
						$base_url = 'https://17th-central-executive-suites.chargify.com/h/3313001/subscriptions/new';
					break;
			}
			break;
			
		case 'silver' :
			$services = 'mail forwarding, Wyoming phone number with unlimited U.S. and Canada long distance, and Wyoming fax number services';
			switch($result->billing_period) {
				case 'month'	:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$base_url = 'https://wyoming-registered-agent.chargify.com/h/3313019/subscriptions/new';
					else if($result->brand == '17th & Central Executive Suites')
						$base_url = 'https://17th-central-executive-suites.chargify.com/h/3313003/subscriptions/new';
					break;
					
				case 'year'		:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$base_url = 'https://wyoming-registered-agent.chargify.com/h/3313020/subscriptions/new';
					else if($result->brand == '17th & Central Executive Suites')
						$base_url = 'https://17th-central-executive-suites.chargify.com/h/3313006/subscriptions/new';
					break;
			}
			break;
			
		case 'gold'	:
			$services = 'mail forwarding, Wyoming phone number with live receptionist, and Wyoming fax number services';
			switch($result->billing_period) {
				case 'month'	:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$base_url = 'https://wyoming-registered-agent.chargify.com/h/3313015/subscriptions/new';
					else if($result->brand  == '17th & Central Executive Suites')
						$base_url = 'https://17th-central-executive-suites.chargify.com/h/3313007/subscriptions/new';
					break;
					
				case 'year'		:
					if($result->brand == 'WyomingRegisteredAgent.com') 
						$base_url = 'https://wyoming-registered-agent.chargify.com/h/3313015/subscriptions/new';
					else if($result->brand == '17th & Central Executive Suites')
						$base_url = 'https://17th-central-executive-suites.chargify.com/h/3313008/subscriptions/new';
					break;
			}
			break;
	}
	$options = '';
	if(isset($base_url)) {
		$url = $base_url . '?first_name='.urlencode($result->first_name).'&last_name='.urlencode($result->last_name).'&email='.urlencode($result->mail).'&reference='.
			$result->uid.'&organization='.urlencode($result->company_name);
		$options .= l('Signup URL', $url).' ';
		unset($base_url);
	} else
		$url = FALSE;
		
	return $url;
}

function wysos_chargify_signup_complete() {
	$admin_email = 'info@wyomingregisteredagent.com';
	
	$customer_id = isset($_GET['customer_id'])?$_GET['customer_id']:'';
	$customer_reference = isset($_GET['customer_reference'])?$_GET['customer_reference']:'';
	$subscription_id = isset($_GET['subscription_id'])?$_GET['subscription_id']:'';
	
	if(isset($_GET['customer_id']) && isset($_GET['customer_reference'])) {
		// Add chargify_customer
		if(db_result(db_query("SELECT COUNT(*) FROM chargify_customers WHERE customer_id=".$_GET['customer_id']." AND uid=".$_GET['customer_reference'])) == 0) {
			$result = db_query("INSERT INTO chargify_customers (customer_id, uid) VALUES (".$_GET['customer_id'].",".$_GET['customer_reference'].")");
		} else {
			mail($admin_email,'ERROR 101 with VO signup',"A client signed up for a virtual office through the Chargify hosted pages, but a matching wra_suites record could not be found "
				."for uid=".$customer_reference." subscription_id=".$subscription_id." customer_id=".$customer_id);
		}
	} else {
		mail($admin_email,'ERROR 102 with VO signup',"A client signed up for a virtual office through the Chargify hosted pages, but a matching wra_suites record could not be found "
			."for uid=".$customer_reference." subscription_id=".$subscription_id." customer_id=".$customer_id);
	}
	if(isset($_GET['customer_reference']) && isset($_GET['subscription_id'])) {
		if(db_result(db_query("SELECT COUNT(*) FROM wra_suites WHERE uid=".$_GET['customer_reference']." AND status IN('active','suspended','wait_for_payment') 
				AND chargify_subscription_id IS NULL")) == 1) {
			// Update chargify_subscription_id
			$result = db_query("UPDATE wra_suites SET chargify_subscription_id=".$_GET['subscription_id'].", pay_by='chargify' WHERE uid=".$_GET['customer_reference']." 
				AND status IN('active','suspended','wait_for_payment') AND chargify_subscription_id IS NULL");
		} else {
			mail($admin_email,'ERROR 103 with VO signup',"A client signed up for a virtual office through the Chargify hosted pages, but a matching wra_suites record could not be found "
				."for uid=".$customer_reference." subscription_id=".$subscription_id." customer_id=".$customer_id);
		}
	} else {
		if(isset($_GET['customer_id']) && user_load($_GET['customer_id'])) {
			// Log user 
			db_query("INSERT INTO wra_user_notes SET uid=".$_GET['customer_reference'].", created_uid=1, category='chargify',notes='ERROR: Cannot find matching wra_suites for new signup for subscription_id=".$_GET['subscription_id'].", created=UNIX_TIMESTAMP()");
			mail($admin_email,'ERROR 104 with VO signup',"A client signed up for a virtual office through the Chargify hosted pages, but a matching wra_suites record could not be found "
				."for uid=".$customer_reference." subscription_id=".$subscription_id." customer_id=".$_GET['customer_id']);
		} else {
			// EMail error
			mail($admin_email,'ERROR 105 with VO signup',"A client signed up for a virtual office through the Chargify hosted pages, but a matching wra_suites record could not be found "
				."for uid=".$customer_reference." subscription_id=".$subscription_id." customer_id=".$customer_id);
		}
	}
	$output = "<b>Thank you!</b><p>Your virtual office signup is complete. You will receive an email within 2 business days with your virtual office instructions.</p>";
	return $output;
}

function wysos_chargify_subscription_form($form,&$form_state){

}













