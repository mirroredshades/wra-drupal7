<?php 
// Create Virtual Office Wizard
function wysos_vo_wiz_add_form($form, &$form_state, $orderid=NULL){
	if(empty($form_state['storage']['step'])) {
		if(is_numeric($orderid)) {
			// Retrieve Order and fill fields
			$order = uc_order_load($orderid);
			if($order) {
				$form_state['storage']['order'] = $order;

				$form_state['storage']['step'] = 2;			
			} else {
				$form_state['storage']['step'] = 1;
			}
		} else {
			$form_state['storage']['step'] = 1;
		}
	}
		
	include_once('wysos.util.inc');

	switch($form_state['storage']['step']) {
		case 1:
			// Get Invoice from user
			drupal_set_title('New Virtual Office : Step 1');
			$form['order_id'] = array(
				'#title' => t('Order Number'),
				'#type' => 'textfield',
				'#size' => '7',
				'#maxlength' => '7',
				'#description' => t('Enter virtual office order number'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['order_id'])? $form_state['storage']['values'][$form_state['storage']['step']]['order_id']:'',
				'#required' => TRUE
			);
			break;

		case 2:
			// Start From Invoice
			$order = $form_state['storage']['order'];
			drupal_set_title('New Virtual Office : Step 2');
			$form['order_id'] = array(
				'#type' => 'item',
				'#value' => isset($form_state['storage']['values'][1]['order_id'])?$form_state['storage']['values'][1]['order_id']:'',
				'#title' => 'Order Number');
			$form['service_level'] = array(
				'#type' => 'select',
				'#title' => 'Service Level',
				'#options' => array( ''=>'', 'bronze' => 'Bronze (Mail Only)','silver' => 'Silver (Mail+Phone+Fax+VoIP)', 'gold'=> 'Gold (Mail+Phone+Receptionist)'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['service_level'])?$form_state['storage']['values'][$form_state['storage']['step']]['service_level']:'',
				'#required' => TRUE
				);
			$form['price'] = array(
				'#type' => 'textfield',
				'#title' => 'Price',
				'#size' => '11',
				'#maxlength' => '12',
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['price'])?$form_state['storage']['values'][$form_state['storage']['step']]['price']:'',
				'#required' => TRUE
				);
			$form['billing_period'] = array(
				'#type' => 'select',
				'#title' => 'Billing Term',
				'#options' => array( ''=>'', 'month' => 'Monthly','year' => 'Annual'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['billing_period'])?$form_state['storage']['values'][$form_state['storage']['step']]['billing_period']:'',
				'#required' => TRUE
				);
			$form['pay_by'] = array(
				'#type' => 'select',
				'#title' => 'Pay By',
				'#options' => array( 'chargify' => 'Chargify','invoice' => 'Invoice', 'paypal_wpp' => 'PayPal Subscription'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['billing_period'])?$form_state['storage']['values'][$form_state['storage']['step']]['billing_period']:'',
				'#required' => TRUE
				);
			$form['first_month_paid'] = array(
				'#type' => 'checkbox',
				'#title' => 'First Month Paid',
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['first_month_paid'])?$form_state['storage']['values'][$form_state['storage']['step']]['first_month_paid']:'',
				);
			$form['user_name'] = array(
				'#title' => t('User Name'),
				'#type' => 'textfield',
				'#size' => '30',
				'#maxlength' => '60',
				'#autocomplete_path' => 'user/autocomplete',
				'#description' => t('Please enter user name.'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['user_name'])?$form_state['storage']['values'][$form_state['storage']['step']]['user_name']:'',
				'#required' => TRUE
			);
			$form['company_name'] = array(
				'#title' => t('Company'),
				'#type' => 'textfield',
				'#size' => '50',
				'#maxlength' => '80',
				'#autocomplete_path' => 'wysos/autocomplete/all_companies',
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['company_name'])?$form_state['storage']['values'][$form_state['storage']['step']]['company_name']:'',
				'#required' => TRUE
			);
			$form['brand'] = array(
				'#type' => 'select',
				'#title' => 'Brand',
				'#options' => array( 'WyomingRegisteredAgent.com'=>'WyomingRegisteredAgent.com','17th & Central Executive Suites'=>'17th & Central Executive Suites'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['brand'])?$form_state['storage']['values'][$form_state['storage']['step']]['brand']:'',
				'#required' => TRUE
				);
			$form['street_address'] = array(
				'#type' => 'select',
				'#title' => 'Street Address',
				'#options' => array( '109 E. 17th St.'=>'109 E. 17th St.','1623 Central Ave.'=>'1623 Central Ave.'),
				'#default_value' => isset($form_state['storage']['values'][$form_state['storage']['step']]['street_address'])?$form_state['storage']['values'][$form_state['storage']['step']]['street_address']:'',
				'#required' => TRUE
				);
			break;
			
		case 3 : 
			$order = $form_state['storage']['order'];
			drupal_set_title('New Virtual Office : Step 3');

			$form['suite_no'] = array(
				'#type' => 'item',
				'#title' => 'Suite No.',
				'#value' => isset($form_state['storage']['suite_no'])?$form_state['storage']['suite_no']:'',
				);
				
			if($form_state['storage']['values'][2]['service_level'] == 'silver' OR $form_state['storage']['values'][2]['service_level'] == 'gold') 
			{
				// Show Phone, Fax Info
				$result = db_query("SELECT did FROM wra_did WHERE did NOT IN (SELECT value FROM wra_suite_resources 
					WHERE resource_type IN('did','fax')) AND MOD(did,2)=0 ORDER BY did LIMIT 50")->fetchAll();
				if (!$result) {
					die('Invalid query');
				}  
			
				if(empty($form_state['values']['phone_no'])) {
					$phone_no[0] = '';
				} else {
					$phone_no[$form_state['values']['phone_no']] = $form_state['values']['phone_no'];
				}
				foreach($result as $rows) {
					$phone_no[$rows->did] = $rows->did;
				}
			
				$form['phone_no'] = array(
					'#title' => t('Phone No'),
					'#type' => 'select',
					'#options' => $phone_no,
					'#default_value' => isset($form_state['storage']['values'][3]['phone_no'])?$form_state['storage']['values'][3]['phone_no']:'',
				);
				$form['vmail2email'] = array(
					'#title' => t('Voicemail to email'),
					'#type' => 'textfield',
					'#size' => '60',
					'#maxlength' => '254',
					'#autocomplete_path' =>  'wysos/email/autocomplete',
					'#default_value' => isset($form_state['storage']['values'][3]['vmail2email'])?$form_state['storage']['values'][3]['vmail2email']:'',
				);
				$result = db_query("SELECT did FROM wra_did 
				WHERE did NOT IN (SELECT value FROM wra_suite_resources WHERE resource_type IN('did','fax')) AND MOD(did,2)=1 ORDER BY did LIMIT 50")->fetchAll();
				if (!$result) {
					die('Invalid query');
				}  
			
				if(empty($form_state['values']['fax_no'])) {
					$fax_no[] = '';
				} else {
					$fax_no[$form_state['values']['fax_no']] = $form_state['values']['fax_no'];
				}
				foreach($result as $rows) {
					$fax_no[$rows->did] = $rows->did;
				}
			
				$form['fax_no'] = array(
					'#title' => t('Fax No'),
					'#type' => 'select',
					'#options' => $fax_no,
					'#default_value' => isset($form_state['storage']['values'][3]['fax_no'])?$form_state['storage']['values'][3]['fax_no']:'',
				);
				$form['fax2email'] = array(
					'#title' => t('Fax to email'),
					'#type' => 'textfield',
					'#size' => '60',
					'#maxlength' => '254',
					'#autocomplete_path' =>  'wysos/email/autocomplete',
					'#default_value' => isset($form_state['storage']['values'][3]['fax2email'])?$form_state['storage']['values'][3]['fax2email']:'',
					'#disabled' => $mode == 'browse' ? TRUE : FALSE
				);
				$form['mac'] = array(
					'#title' => t('VoIP MAC'),
					'#type' => 'textfield',
					'#size' => '12',
					'#maxlength' => '12',
					'#default_value' => isset($form_state['storage']['values'][3]['mac'])?$form_state['storage']['values'][3]['mac']:'',
				);
			}
			if($form_state['storage']['values'][2]['billing_period'] == 'month' && $form_state['storage']['values'][2]['pay_by'] == 'chargify') {
				$newdate = strtotime ( '+1 month') ;
			
				$form['next_billing_date'] = array(	
					'#type' => 'date',
					'#title' => t('Next Billing Date'),
					'#default_value' => array(
						'month' => empty($form_state['storage']['values'][3]['next_billing_date']['month']) ? format_date($newdate, 'custom', 'n') : $form_state['storage']['values'][3]['next_billing_date']['month'], 
						'day' => empty($form_state['storage']['values'][3]['next_billing_date']['day']) ? format_date($newdate, 'custom', 'j') : $form_state['storage']['values'][3]['next_billing_date']['day'], 
						'year' => empty($form_state['storage']['values'][3]['next_billing_date']['year']) ? format_date($newdate, 'custom', 'Y') : $form_state['storage']['values'][3]['next_billing_date']['year'],
					  ),
				);
				$form['payment'] = array(
					'#title' => t("Payment"),
					'#prefix' => '<div id="payment">',
					'#suffix' => '</div>',
					'#type' => 'fieldset',
					'#collapsible' => TRUE, 
					'#collapsed' => FALSE
				);
				$form['payment']['cc_number'] = array(
					'#type' => 'textfield',
					'#title' => t('Card number'),
				//	'#default_value' => $_SESSION['clear_cc'] ? '' : $default_num,
					'#default_value' => $form_state['storage']['values'][3]['cc_number'],
					'#attributes' => array('autocomplete' => 'off'),
					'#size' => 20,
					'#maxlength' => 19,
				);
				$form['payment']['cc_exp_month'] = uc_select_month(t('Expiration Month'), $form_state['storage']['values'][3]['cc_exp_month']);
				$form['payment']['cc_exp_year'] = uc_select_year(t('Expiration Year'), $form_state['storage']['values'][3]['cc_exp_year']);
				
				$form['payment']['billing_first_name'] = array(
					'#title' => t('First Name'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '254',
					'#default_value' => isset($form_state['storage']['values'][3]['billing_first_name'])?$form_state['storage']['values'][3]['billing_first_name']:'',
			 );
				$form['payment']['billing_last_name'] = array(
					'#title' => t('Last Name'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '254',
					'#default_value' => isset($form_state['storage']['values'][3]['billing_last_name'])?$form_state['storage']['values'][3]['billing_last_name']:'',
				);
				$form['payment']['billing_street1'] = array(
					'#title' => t('Address'),
					'#type' => 'textfield',
					'#size' => '60',
					'#maxlength' => '254',
					'#default_value' => isset($form_state['storage']['values'][3]['billing_street1'])?$form_state['storage']['values'][3]['billing_street1']:'',
				);
				$form['payment']['billing_street2'] = array(
					'#type' => 'textfield',
					'#size' => '60',
					'#maxlength' => '254',
					'#default_value' => isset($form_state['storage']['values'][3]['billing_street2'])?$form_state['storage']['values'][3]['billing_street2']:'',
				);
				$form['payment']['billing_city'] = array(
					'#title' => t('City'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '254',
					'#default_value' => isset($form_state['storage']['values'][3]['billing_city'])?$form_state['storage']['values'][3]['billing_city']:'',
				);
			
				if (isset($form_state['storage']['values'][3]['billing_country'])) {
					$country_id = intval($form_state['storage']['values'][3]['billing_country']);
				} else {
					$country_id = NULL;
				}
				//$form['payment']['billing_country'] = uc_country_select(uc_get_field_name('country'),$country_id);
				//$form['payment']['billing_zone'] = uc_zone_select(uc_get_field_name('zone'), $form_state['storage']['values'][3]['billing_zone'], NULL, $country_id);

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
					'#default_value' => $form_state['storage']['values'][3]['billing_postal_code'],
				);
			}
			break;

		case 4 : 
			$order = $form_state['storage']['order'];
			drupal_set_title('New Virtual Office : Complete');
			$fid = wysos_virtual_office_instructions($form_state['storage']['suite_id'], false);
			$did = db_query("SELECT did FROM wra_download_files WHERE fid=".$fid)->fetchField();
			switch($form_state['storage']['values'][2]['service_level']) {
				case 'bronze' :
					$form[] = array(
						'#type' => 'item',
						'#description' => 'Programming complete. Download the '.l('instructions','https://my.wyomingregisteredagent.com/wysos/download/'.$did.'/'.$fid)
					);
					break;

				case 'silver' :
				case 'gold' :
					$form[] = array(
						'#type' => 'item',
						'#description' => 'Programming complete. <ul><li>Download the '.l('instructions','https://my.wyomingregisteredagent.com/wysos/download/'.$did.'/'.$fid).'</li></ul>'
					);
						$form[] = array(
							'#type' => 'item',
							'#title' => 'VoIP Adapter Programming',
							'#description' => '<ol><li>Plug in the adapter to power, network and telephone</li>
								<li>Pick up the phone and dial your phone number</li>
								<li>SIP User ID: <b>'.$form_state['storage']['sip_user'].'</b></li>
								<li>Authenticate Password: <b>'.$form_state['storage']['sip_password'].'</b></li>
								</ol>'
						);
				break;		
			}
			break;
	}

	if($form_state['storage']['step'] > 1) {
		$form['previous'] = array(
			'#type' => 'submit',
			'#value' => '<< Previous' 
			);
	}
	
	if($form_state['storage']['step'] < 4) {
		$form['next'] = array(
			'#type' => 'submit',
			'#value' => 'Next >>'
			);
	}

	return ($form);
}

function wysos_vo_wiz_add_form_validate($form, &$form_state)
{
	switch($form_state['storage']['step']) {
		case 1 :
			$order = uc_order_load($form_state['values']['order_id']);
			if(!$order) {
				form_set_error('order_id','Order does not contain virtual office');
			}
			break;

		case 2 :
			break;

		case 3 :
			if($form_state['storage']['values'][2]['billing_period'] == 'month' && $form_state['storage']['values'][2]['pay_by'] == 'chargify') {
				include_once('wysos.pay.inc');
				_wysos_cc_validate($form_state);
			}
			break;
	}
}

function wysos_vo_wiz_add_form_submit($form, &$form_state)
{
	global $user;
	require_once('wysos.util.inc');
	require_once('wysos.virtualoffice.inc');
	$form_state['storage']['values'][$form_state['storage']['step']] = $form_state['values'];
	
	 
	
	switch($form_state['storage']['step']) {
		case 1 :
			$order = uc_order_load($form_state['values']['order_id']);
			$form_state['storage']['order'] = $order;
			$form_state['storage']['step'] = 2;			
			$form_state['rebuild'] = TRUE;
			break;

		case 2 :
			// Retrieve IDs
			$uid = db_query("SELECT uid FROM users WHERE name=:name", array(':name'=>$form_state['storage']['values'][2]['user_name']))->fetchField();
			$cid = db_query("SELECT company_id FROM wra_search_all_companies WHERE company_name=:companyname", array(':companyname'=>mysql_real_escape_string($form_state['storage']['values'][2]['company_name'])))->fetchField();
			if($cid == 0) {
				// Company Not Registered, so Add
				$cid = db_insert('wra_companies')
						->fields(array(
							'company_name' => mysql_real_escape_string($form_state['storage']['values'][2]['company_name']),
							'uid' => $uid,
							'filing_type' => 'Other',
							'created'=>REQUEST_TIME,
							'updated'=>REQUEST_TIME,
						))->execute();
			
				//$cid = db_last_insert_id('wra_companies','company_id');
			}
			$form_state['storage']['company_id']=$cid;
			$form_state['storage']['uid'] = $uid;
			// Create wra_suites record
			switch($form_state['storage']['values'][2]['billing_period']) {
				case 'month' :
					$anniv =  date('j');
					if ($anniv > 28)
						$anniv = 28;
				break;
					
				case 'year' :
					$anniv =  date('n');
					break;

				case 'quarter' :
					$anniv =  date('n');
					break;

				case 'bi-annual' :
					$anniv =  date('n');
					break;
			}
				
			if(isset($form_state['storage']['suite_id'])) {
				// Update wra_suite
				/*$query = sprintf("UPDATE wra_suites SET uid=%d,company_id=%d,company_name='%s',street_address='%s',brand='%s', price=%f, billing_period='%s', service_level='%s', 
					orig_order_id=%d, anniv=%d, pay_by='%s'
					WHERE suite_id=%d", 
					$uid, $cid, $form_state['storage']['values'][2]['company_name'],  $form_state['storage']['values'][2]['street_address'], 
					$form_state['storage']['values'][2]['brand'], $form_state['storage']['values'][2]['price'], $form_state['storage']['values'][2]['billing_period'],
					$form_state['storage']['values'][2]['service_level'], $form_state['storage']['values'][1]['order_id'], $anniv, $form_state['storage']['suite_id'], $form_state['storage']['values'][2]['pay_by']);*/
					
					$query = db_update('wra_suites')
						->fields(array(
							'uid' => $uid,
							'company_id' => $cid,
							'company_name' => $form_state['storage']['values'][2]['company_name'],
							'street_address' => $form_state['storage']['values'][2]['street_address'],
							'brand' => $form_state['storage']['values'][2]['brand'],
							'price' => $form_state['storage']['values'][2]['price'],
							'billing_period' => $form_state['storage']['values'][2]['billing_period'],
							'service_level' => $form_state['storage']['values'][2]['service_level'],
							'orig_order_id' => $form_state['storage']['values'][1]['order_id'],
							'anniv' => $anniv,
							'pay_by' => $form_state['storage']['values'][2]['pay_by'],
						))->condition('suite_id',$form_state['storage']['suite_id'],'=')->execute();
					

				//db_query($query);
				//drupal_set_message('query1: ');
			} else {
				// Insert new wra_suite
				/*
				$query = sprintf("INSERT INTO wra_suites (uid, company_id, company_name, street_address, suite_no, brand, price, billing_period, pay_by, anniv, service_level, status, orig_order_id, created, created_uid)
					VALUES (%d, %d, '%s', '%s', (SELECT MAX(s.suite_no)+1 FROM wra_suites s), '%s', %f, '%s', '%s', %d, '%s', 'order_entry', %d, UNIX_TIMESTAMP(), %d)", 
					$uid, $cid, $form_state['storage']['values'][2]['company_name'], $form_state['storage']['values'][2]['street_address'], 
					$form_state['storage']['values'][2]['brand'], $form_state['storage']['values'][2]['price'], $form_state['storage']['values'][2]['billing_period'], 
					$form_state['storage']['values'][2]['pay_by'], $anniv,
					$form_state['storage']['values'][2]['service_level'], $form_state['storage']['values'][1]['order_id'], $user->uid);
					
					*/
					$suite_no = db_query("SELECT MAX(suite_no)+1 FROM wra_suites")->fetchField();
					//print_r($suite_no);
					//die();
				$query = db_insert('wra_suites')
						->fields(array(
							'uid' => $uid,
							'company_id' => $cid,
							'company_name' => $form_state['storage']['values'][2]['company_name'],
							'street_address' => $form_state['storage']['values'][2]['street_address'],
							'suite_no' => $suite_no,
							'brand' => $form_state['storage']['values'][2]['brand'],
							'price' => $form_state['storage']['values'][2]['price'],
							'billing_period' => $form_state['storage']['values'][2]['billing_period'],
							'pay_by' => $form_state['storage']['values'][2]['pay_by'],
							'anniv' => $anniv,
							'service_level' => $form_state['storage']['values'][2]['service_level'],
							'status' => 'order_entry',
							'orig_order_id' => $form_state['storage']['values'][1]['order_id'],
							'created' => REQUEST_TIME,
							'created_uid' => $user->uid,
						))->execute();
				
				//db_query($query);
//				drupal_set_message('query2: '.$query);
				$form_state['storage']['suite_id'] = $query;
				$form_state['storage']['suite_no'] = db_query("SELECT suite_no FROM wra_suites WHERE suite_id=:suite_id",array(':suite_id'=>$form_state['storage']['suite_id']))->fetchField(); 
			}
			// Update suite_id
			//db_query("UPDATE wra_companies SET suite_id=".$form_state['storage']['suite_id']." WHERE company_id=".$cid);
			
			db_update('wra_companies')
				->fields(array(
					'suite_id' => $form_state['storage']['suite_id'],
				))->condition('company_id', $cid, '=')->execute();
			
			
			if($form_state['clicked_button']['#id'] == 'edit-previous') {
				if($form_state['storage']['step'] > 1) {
					$form_state['storage']['step']--;
				};
			} elseif($form_state['clicked_button']['#id'] == 'edit-next') {
				if($form_state['storage']['step'] < 4) {
					$form_state['storage']['step']++;
				}
			};
			$form_state['rebuild'] = TRUE;
			break;

		case 3 :
			$download_url = 'wysos/download/';
			$download_path = $_SERVER['DOCUMENT_ROOT'].'/public/';

			//$result = db_query("DELETE FROM wra_suite_resources WHERE suite_id=:suite_id", array(':suite_id'=>$form_state['storage']['suite_id']));
			
			$result = db_delete('wra_suite_resources')
						->condition('suite_id',$form_state['storage']['suite_id'])
						->execute();
						
			switch($form_state['storage']['values'][2]['brand']) {
				case 'WyomingRegisteredAgent.com'	:
					$brand = 'WRA';
					break;
					
				case '17th & Central Executive Suites' :
					$brand = 'SAC';
					break;
			}
			if($form_state['storage']['values'][2]['billing_period'] == 'month' && $form_state['storage']['values'][2]['pay_by'] == 'chargify') {
				// Check if chargify customer is created yet
				$uid = db_query("SELECT uid FROM users WHERE name=:name", array(':name' =>$form_state['storage']['values'][2]['user_name']))->fetchField();

				if(db_query("SELECT COUNT(*) FROM chargify_customers WHERE uid=".$uid)->fetchField() > 0) {
					$row_customer = db_query("SELECT * FROM chargify_customers WHERE uid=".$uid)->fetchField();
					//$row_customer = db_fetch_array($result);
					$chargify_customer_id = $row_customer->customer_id;
				} else {
					// Create Chargify User
					$row = db_query("SELECT u.mail, a.*, z.zone_code AS state, c.country_iso_code_2 AS country 
						FROM users u
						INNER JOIN uc_addresses a ON a.uid = u.uid
						INNER JOIN uc_addresses_defaults d ON a.uid=d.uid 
						LEFT JOIN uc_zones z ON a.zone=z.zone_id 
						LEFT JOIN uc_countries c ON a.country = c.country_id WHERE u.uid=".$uid)->fetchAll();
					//$row = db_fetch_array($result);
					
					$array_customer = array(
						'first_name' => $row->first_name,
						'last_name' => $row->last_name,
						'email' => $row->mail,
						'organization' => $row->company,
						'reference' => $uid,
						);
//		drupal_set_message('array_customer: <pre>'.print_r($array_customer, true).'</pre>');			
						
					// creating object of SimpleXMLElement
					$xml_customer = new SimpleXMLElement("<?xml version=\"1.0\"?><customer></customer>");
					
					// function call to convert array to xml
					include_once('wysos.chargify.inc');
					array_to_xml($array_customer,$xml_customer);
					file_put_contents("/home/wyregnet/xml_out3.txt", $xml_customer->asXML());
					
					//saving generated xml file
					$customer = chargify_api_customer_create($xml_customer->asXML());
					if(isset($customer->error)) {
						drupal_set_message('customer error: '.print_r($customer, true));
						return;
					} else {
//						drupal_set_message('new customer: '.print_r($customer, true));
						$chargify_customer_id = $customer->getID();
						// Save the Customer
						if(db_query("SELECT COUNT(*) FROM chargify_customers WHERE customer_id=".$customer->getID()." AND uid=".$form_state['storage']['uid'])->fetchField() > 0) {
							// Change Existing
							
						} else {
							// Create New
							//$result = db_query("INSERT chargify_customers (customer_id, uid) VALUES (".$customer->getID().",".$form_state['storage']['uid'].")");
							
							$result = db_insert('chargify_customers')
									->fields(array(
										'customer_id'=> $customer->getID(),
										'uid' => $form_state['storage']['uid'],
									))->execute();
							
		//					chargify_user_save($customer);
						}
					}
				}
				// Create Chargify Subscription
				switch($form_state['storage']['values'][2]['service_level']) {
					case 'bronze' :
						if($form_state['storage']['values'][2]['first_month_paid'])
							$product_handle = 'vobrmf';
						else
						 	$product_handle = 'vobrm'; 
						break;
						
					case 'silver' :
						if($form_state['storage']['values'][2]['first_month_paid'])
							$product_handle = 'vosimf';
						else
							$product_handle = 'vosim';
						break;
						
					case 'gold' :
						if($form_state['storage']['values'][2]['first_month_paid'])
							$product_handle = 'vogomf';
						else
							$product_handle = 'vogom';
						break;
				}
				$billing_state = db_query("SELECT zone_code FROM uc_zones WHERE zone_id=".$form_state['storage']['values'][3]['billing_zone'])->fetchField();
				$array_subscription = array(
					'product_handle' => $product_handle,
					'customer_id' => $chargify_customer_id,
					'credit_card_attributes' => array(
						'first_name' => $form_state['storage']['values'][3]['billing_first_name'],
						'last_name' => $form_state['storage']['values'][3]['billing_last_name'],
	//						'billing_address' => $form_state['values']['billing_address'],
						'billing_address' => empty($form_state['storage']['values'][3]['billing_street2']) ? $form_state['storage']['values'][3]['billing_street1'] : 
							$form_state['storage']['values'][3]['billing_street1'] . '\n'.$form_state['storage']['values'][3]['billing_street2'],
						'billing_city' => $form_state['storage']['values'][3]['billing_city'],
						'billing_state' => $billing_state,
						'billing_zip' => $form_state['storage']['values'][3]['billing_postal_code'],
						'billing_country' => $form['payment']['billing_country']['#options'][$form_state['values']['billing_country']],
						'full_number' => $form_state['storage']['values'][3]['cc_number'],
						'expiration_month' => $form_state['storage']['values'][3]['cc_exp_month'],
						'expiration_year' => $form_state['storage']['values'][3]['cc_exp_year']),
					'next_billing_at' => $form_state['storage']['values'][3]['next_billing_date']['year'].'-'.$form_state['storage']['values'][3]['next_billing_date']['month'].'-'.$form_state['storage']['values'][3]['next_billing_date']['day']
					);
//	drupal_set_message('array_subscription: <pre>'.print_r($array_subscription, true).'</pre>');			
					
				// creating object of SimpleXMLElement
				$xml_subscription = new SimpleXMLElement("<?xml version=\"1.0\"?><subscription></subscription>");
	
				// function call to convert array to xml
				include_once('wysos.chargify.inc');
				array_to_xml($array_subscription,$xml_subscription);
				$xml_subscription->asXML("/home/wyregent/xml_out.txt");
				file_put_contents("/home/wyregnet/xml_out1.txt", $xml_subscription->asXML());
				
				//saving generated xml file
				$subscription = chargify_api_subscription_create($xml_subscription->asXML());
				if(isset($subscription->error)) {
					drupal_set_message('subscription error: '.print_r($subscription, true));
				} else {
//					drupal_set_message('new subscription: '.print_r($subscription, true));
					// Save the Subscription ID
	
					//$result = db_query("UPDATE wra_suites SET chargify_subscription_id=".$subscription->getId()." WHERE suite_id=".$form_state['storage']['suite_id']);
					
					$result = db_update('wra_suites')
							->fields(array(
								'chargify_subscription_id' => $subscription->getId(),
							))->condition('suite_id',$form_state['storage']['suite_id'])->execute();
				}
			}
			
			switch($form_state['storage']['values'][2]['service_level']) {
				case 'bronze' :
					$slvl = 'Bronze';
					// Set Globals
					$GLOBALS['company_name'] = $form_state['storage']['values'][2]['company_name'];
					$GLOBALS['street_address'] = $form_state['storage']['values'][2]['street_address'];
					$GLOBALS['suite_no'] = $form_state['storage']['suite_no'];
					$GLOBALS['user_name'] = $form_state['storage']['values'][2]['user_name'];
					
					/*db_query("INSERT wra_suite_resources (suite_id, resource_type, value, created) VALUES (%d,'extension','%s',UNIX_TIMESTAMP())", 
						$form_state['storage']['suite_id'],	$form_state['storage']['suite_no']);
						*/
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'extension',
								'value' => $form_state['storage']['suite_no'],
								'created' => REQUEST_TIME,
							))->execute();
					break;
					
				case 'silver' :
				case 'gold' :
					if($form_state['storage']['values'][2]['service_level'] == 'silver') {
						$slvl = 'Silver';
						$data = array( 'incoming_action' => 'sip',
							'fwd_phone_num' => '' );
							
						/*db_query("INSERT wra_suite_resources (suite_id, resource_type, value, data, created) VALUES (%d,'did','%s','%s',UNIX_TIMESTAMP())", 
							$form_state['storage']['suite_id'], $form_state['storage']['values'][3]['phone_no'], serialize($data)); */
							//$data1 = serialize($data);
						db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'did',
								'value' => $form_state['storage']['values'][3]['phone_no'],
								'data' => $data,
								'created' => REQUEST_TIME,
							))->execute();	
							
					} elseif($form_state['storage']['values'][2]['service_level'] == 'gold') {
						$slvl = 'Gold';
						$data = array( 'incoming_action' => 'sip',
							'fwd_phone_num' => '' );
						//$data1 = serialize($data);
						db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'did',
								'value' => $form_state['storage']['values'][3]['phone_no'],
								'data' => $data,
								'created' => REQUEST_TIME,
							))->execute();		
							
					}
					
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'mac',
								'value' => $form_state['storage']['values'][3]['mac'],
								'created' => REQUEST_TIME,
							))->execute();
						
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'fax',
								'value' => $form_state['storage']['values'][3]['fax_no'],
								'created' => REQUEST_TIME,
							))->execute();
					
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'vmail2email',
								'value' => $form_state['storage']['values'][3]['vmail2email'],
								'created' => REQUEST_TIME,
							))->execute();
					
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'fax2email',
								'value' => $form_state['storage']['values'][3]['fax2email'],
								'created' => REQUEST_TIME,
							))->execute();
					
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'extension',
								'value' => $form_state['storage']['suite_no'],
								'created' => REQUEST_TIME,
							))->execute();
					
					$sip_user = generatePassword(10,2);
					$sip_password = generatePassword(10,2);
					$form_state['storage']['sip_user']=$sip_user;
					$form_state['storage']['sip_password']=$sip_password;
					
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'sip_user',
								'value' => $sip_user,
								'created' => REQUEST_TIME,
							))->execute();
					
					db_insert('wra_suite_resources')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
								'resource_type' => 'sip_password',
								'value' => $sip_password,
								'created' => REQUEST_TIME,
							))->execute();
					
					wysos_virtual_office_program($form_state['storage']['suite_id']);
					
					$result = db_update('wra_companies')
							->fields(array(
								'suite_id' => $form_state['storage']['suite_id'],
							))->condition('company_id',$form_state['storage']['company_id'])->execute();
					
					//$result = db_query("UPDATE wra_companies SET suite_id=%d WHERE company_id=%d", $form_state['storage']['suite_id'], $form_state['storage']['company_id']); 
					
					
					$result = db_update('wra_suites')
							->fields(array(
								'status' => 'active',
								'contract_start' => REQUEST_TIME,
							))->condition('suite_id',$form_state['storage']['suite_id'])->execute();
					
					include_once('wysos.vo_util.php');
					wysos_vo_util_make_config($form_state['storage']['suite_id']);
					break;
			}	
			
			if($form_state['clicked_button']['#id'] == 'edit-previous') {
				if($form_state['storage']['step'] > 1) {
					$form_state['storage']['step']--;
				};
			} elseif($form_state['clicked_button']['#id'] == 'edit-next') {
				if($form_state['storage']['step'] < 4) {
					$form_state['storage']['step']++;
				}
			};
			$form_state['rebuild'] = TRUE;
			break;
		};
}
