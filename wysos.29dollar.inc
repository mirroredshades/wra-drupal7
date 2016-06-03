<?php
function wysos_29dollar_new_user_form($form, &$form_state) {
	//include_once('wysos.util.inc');
	drupal_set_title('Enter New 29DollarAgent.com User');
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
	$form['mail_fwd'] = array(
		'#title' => t('Mail Forwarding'),
        '#type' => 'checkbox',
	);
	
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => t('Submit New User')
	);
	return($form);
}
function wysos_29dollar_new_user_form_submit($form, &$form_state) {
	$uid = db_query("SELECT uid FROM users WHERE mail= :email",array(':email'=>$form_state['values']['email']))->fetchField();
	
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
		user_multiple_role_edit(array($nuser->uid), 'add_role', 9);	
		$client = user_load($uid);	
	}
	else {
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
	else {
		$result = db_insert('wra_companies')
				->fields(array(
					'company_name' => $form_state['values']['company_name'],
					'filing_type' => 'Other',
					'uid' => $uid,
					'created' => REQUEST_TIME,
					'updated' => REQUEST_TIME,
				))->execute();
	}
	$r_address = db_query('SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
		FROM users
		INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
		INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
		WHERE users.uid ='.$uid);
	
	if($form_state['values']['mail_fwd'])
	{
		// Add Mail Forwarding Role to user
		//$result = db_query("INSERT INTO {users_roles} (uid,rid) VALUES (%d,22)", $uid);
		user_multiple_role_edit(array($nuser->uid), 'add_role', 22);
		drupal_set_message($form_state['values']['company_name'].' for user '.$nuser->name .' with MAIL FORWARDING Added.');
	} else {
		drupal_set_message($form_state['values']['company_name'].' for user '.$nuser->name .' Added.');
	}
		
}
