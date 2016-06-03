<?php 
function wysos_incorp_mail_only_form($form, &$form_state){
	include_once('wysos.util.inc');
	drupal_set_title('Incorp Services Mail Forwarding Client');
	$form['customer_info']['email'] = array(
		'#title' => t("Email"),
		'#type' => 'textfield',
		'#size' => '60',
		'#default_value' => isset($form_state['values']['email'])?$form_state['values']['email']:'',
	);
	$form['customer_info']['first_name'] = array(
		'#title' => t('Customer First Name'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['first_name'])?$form_state['values']['first_name']:'',
	);
	$form['customer_info']['last_name'] = array(
		'#title' => t('Customer Last Name'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['last_name'])?$form_state['values']['last_name']:'',
	);
	$form['customer_info']['company_name'] = array(
		'#title' => t('Company Name'),
        '#type' => 'textfield',
        '#size' => '50',
		'#maxlength' => '80',
        '#autocomplete_path' => 'wysos/autocomplete/all_companies',
	);
	$form['customer_info']['street1'] = array(
		'#title' => t('Address'),
		'#type' => 'textfield',
		'#size' => '60',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['street1'])?$form_state['values']['street1']:'',
	);
	$form['customer_info']['street2'] = array(
		'#type' => 'textfield',
		'#size' => '60',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['street2'])?$form_state['values']['street2']:'',
	);
	$form['customer_info']['city'] = array(
		'#title' => t('City'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['city'])?$form_state['values']['city']:'',
	);
	//$form['customer_info']['country'] = uc_country_select(uc_get_field_name('country'),$form_state['values']['country'],NULL,NULL,FALSE);
	//$form['customer_info']['zone'] = uc_zone_select(uc_get_field_name('zone'), $form_state['values']['zone'],NULL, $form_state['values']['country'],NULL,FALSE);
	$countries = db_query("SELECT country_id,country_name FROM uc_countries")->fetchAll();
	$country_list = array(); 
	foreach($countries as $country){
		$country_list[$country->country_id] = $country->country_name;
	}
	$form['customer_info']['country'] = array(
		'#type' => 'select',
		'#options' => $country_list,
		'#title' => t('Country'),
		'#id' => 'tcountry', 
	);
	$zones = db_query("SELECT zone_id, zone_name FROM uc_zones")->fetchAll();
	$zone_list = array();
	foreach($zones as $zone){
		$zone_list[$zone->zone_id] = $zone->zone_name;
	}
	$form['customer_info']['zone'] = array(
		'#type' => 'select',
		'#options' => $zone_list,
		'#title' => t('State/Province'),
		'#id' => 'tzone', 
	);
	$form['customer_info']['zone_load'] = array(
		'#type' => 'item',
	);
	$form['customer_info']['postal_code'] = array(
		'#title' => t('Zip/Postal Code'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '40',
		'#default_value' => isset($form_state['values']['postal_code'])?$form_state['values']['postal_code']:'',
	);
	$form['customer_info']['phone'] = array(
		'#title' => t('Phone Number'),
		'#type' => 'textfield',
		'#size' => '20',
		'#maxlength' => '20',
		'#default_value' => isset($form_state['values']['phone'])?$form_state['values']['phone']:'',
	);
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => t('Submit Order')
	);
	return($form);
}

function wysos_incorp_mail_only_form_submit($form, &$form_state) {
	$uid = db_query("SELECT uid FROM users WHERE mail= :email",array('email'=>$form_state['values']['email']))->fetchField();
	
	if($uid == FALSE || $uid == 0){
		$uname = strtolower(substr(trim($form_state['values']['first_name']),0,1).trim($form_state['values']['last_name']));
		$baseuname = $uname;
		if(db_query("SELECT count(name) FROM {users} WHERE name = :name",array(':name'=>$uname))->fetchField() > 0){
			$uname = $baseuname. mt_rand(0,999);
		}
		$newuser = array(
			'name' => $uname,
			'pass' => $uname.'11',
			'mail' => $form_state['values']['email'],
			'status' => 1,
			'init' => $form_state['values']['email']
		);
		$nuser = user_save(NULL,$newuser);
		$country = isset($form_state['values']['country'])?$form_state['values']['country']:'';
		$zone = isset($form_state['values']['zone'])?$form_state['values']['zone']:'';
		$result = db_insert('uc_addresses')
					->fields(array(
						'uid' => $nuser->uid,
						'first_name' => $form_state['values']['first_name'],
						'last_name' => $form_state['values']['last_name'],
						'phone' => $form_state['values']['phone'],
						'company' => $form_state['values']['company_name'],
						'street1' => $form_state['values']['street1'],
						'street2' => $form_state['values']['street2'],
						'city' => $form_state['values']['city'],
						'zone' => $zone,
						'postal_code' => $form_state['values']['postal_code'],
						'country' => $country,
						'address_name' => 'Mailing',
						'created' => REQUEST_TIME,
						'modified' => REQUEST_TIME,
					))->execute();
		$aid = 	$result;
		$uid = 	$nuser->uid;
		$uad = db_insert('uc_addresses_defaults')
				->fields(array(
					'aid' => $result,
					'uid' => $nuser->uid,				
				))->execute();
	
		// Add Customer Role to user
		$result = db_query("INSERT INTO {users_roles} (uid,rid) VALUES (%d,9)", $uid);
	
		$client = user_load($uid);
	} else {
		// Load Existing User
		$client = user_load($uid);
	}
	
	$cid = db_query("SELECT company_id FROM {wra_companies} WHERE company_name = :cname",array(':cname'=>$form_state['values']['company_name']))->fetchField();
	if($cid > 0){
		$result = db_update('wra_companies')
			->fields(array('uid'=>$uid))
			->condition('company_id',$cid, '=')
			->execute();
	}
	else{
		$result = db_insert('wra_companies')
			->fields(array(
				'company_name' => $form_state['values']['company_name'],
				'filing_type' => 'Other',
				'uid' => $uid,
				'created' => REQUEST_TIME,
				'updated' => REQUEST_TIME,
			))->execute();
	}
	$address = db_query('SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
		FROM users
		INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
		INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
		WHERE users.uid ='.$uid)->execute()->fetchAll();
	
	//$address = db_fetch_array($r_address);
	// Create the Order
	$result = db_insert('uc_orders')
				->fields(array(
					'uid' => '714',
					'order_status' => 'pending',
					'order_total' => 95,
					'product_count' => 1,
					'primary_email' => 'karolyn.knight@incorp.com',
					'delivery_first_name' => $address->first_name,
					'delivery_last_name' => $address->last_name,
					'delivery_phone' => $address->phone,
					'delivery_company' => $form_state['values']['company_name'],
					'delivery_street1' => $address->street1,
					'delivery_street2' => $address->street2,
					'delivery_city' => $address->city,
					'delivery_zone' => $address->zone,
					'delivery_postal_code' => $address->postal_code,
					'delivery_country' => $address->country,
					'billing_first_name' => 'Karolyn',
					'billing_last_name' => 'Knight',
					'billing_phone' => '702.866.2500 ext. 6182',
					'billing_company' => 'INCORP Services',
					'billing_street1' => '2360 Corporate Circle, Suite 400',
					'billing_street2' => '',
					'billing_city' => 'Henderson',
					'billing_zone' => 39,
					'billing_country' => '89074-7722',
					'payment_method' => '840',
					'created' => REQUEST_TIME,
					'host' => '',	
				))->execute();
	
	if(!$result) {
		return;
	}
	$orderid = $result;
	
	$suiteno = db_query("SELECT MAX(suite_no)+1 FROM wra_suites");
	
	$suite_id = db_insert('wra_suites')
				->fields(array(
				'company_id' => $cid,
				'company_name' => $form_state['values']['company_name'],
				'street_address' => '109 E. 17th St.',
				'suite_no' => $suiteno,
				'mail_status' => 'mail_forward',
				'uid' => $client->uid,
				'parent_uid' => 714,
				'anniv' => date('n'),
				'created' => REQUEST_TIME,
				'price' => 95,
				'pay_by' => 'invoice',
				'billing_period' => 'year',
				'contract_start' => REQUEST_TIME,
				'status' => 'active',
				'orig_order_id' => $orderid,
				'comment' => 'Incorp Services Client',
				))->execute();

	// Ad uc_order_products
	$data = array("attributes" => array("Suite Number"=>array($suiteno),
			"Company Name" => $form_state['values']['company_name']));
	
	$result	= db_insert('uc_order_products')
			->fields(
				array(	
					'order_id' => $orderid,
					'nid' => 1261,
					'model' => 'WYMF',
					'title' => 'Mail Forwarding Only-1 Year',
					'qty' => 1,
					'cost' => 95.00,
					'price' => 95.00,
					'data' => serialize($data),
			))->execute();
	// Update the Company with the new suite id
		
	$result = db_update('wra_companies')
			->fields(array('suite_id'=>$suiteid))
			->condition('company_id',$cid,'')
			->execute();
	
	drupal_set_message('Suite '.$suiteno.' Added for '.$form_state['values']['company_name']);
		
}
