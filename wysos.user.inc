<?php

function wysos_user_form($form,&$form_state,$uid=NULL) {
	include_once('wysos.util.inc');

		if(!is_numeric($uid)) {
			drupal_access_denied();
			return false;
		}
		$form_state['storage']['uid'] = $uid;
		// Load user record
		$userObj = user_load($uid);
		//profile_load_profile($userObj);
		$form_state['storage']['user'] = get_object_vars($userObj);
		drupal_set_title('Browse User '.$form_state['storage']['user']['name']);
		$form_state['values'] = (array) $form_state['storage']['user'];
		
		// Load Companies
		$form_state['storage']['company'] = array();
		$form_state['storage']['company']['company_name'] = '';
		$result = db_query('SELECT company_id, company_name, filing_num, eFNum, filing_date, sos_status FROM wra_companies WHERE uid=:uid ORDER BY company_name', array(':uid'=>$uid))->fetchAll();
		
		foreach($result as $rowCo) {
			$form_state['storage']['company'][$rowCo->company_id] = $rowCo;
		}		
		
		// Load Names / Addresses
		$form_state['storage']['address'] = array();
		$result = db_query("SELECT uc_addresses.*, uc_zones.zone_code AS state, uc_countries.country_name AS country_name  FROM uc_addresses 
		INNER JOIN uc_zones ON uc_addresses.zone = uc_zones.zone_id
		INNER JOIN uc_countries ON uc_countries.country_id = uc_addresses.country
        LEFT JOIN uc_addresses_defaults ON uc_addresses.aid = uc_addresses_defaults.aid
		WHERE uc_addresses.uid=:uid
		ORDER BY uc_addresses_defaults.aid DESC",array(':uid'=>$uid))->fetchAll();
		
		foreach($result as $rowAddress) {
			$form_state['storage']['address'][$rowAddress->aid] = $rowAddress;
		}
		// Load wra_suites records
		$form_state['storage']['suite'] = array();
		$result = db_query('SELECT wra_suites.* FROM wra_suites WHERE uid=:uid',array(':uid'=>$uid))->fetchAll();
		foreach($result as $rowSuite) {
			$form_state['storage']['suite'][$rowSuite->suite_id] = $rowSuite;
		}
		
		// Load Offices
		$form_state['storage']['office'] = array();
		$result = db_query("SELECT wra_offices.*, wra_office_suite.office_no FROM wra_offices INNER JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no WHERE wra_office_suite.suite_id IN(SELECT suite_id FROM wra_suites WHERE uid=:uid)",array(':uid'=>$uid))->fetchAll();
		foreach($result as $rowOffice) {
			$form_state['storage']['office'][$rowOffice->office_no] = $rowOffice;
		}
		
		// Load Orders
		$form_state['storage']['orders'] = array();
		if($form_state['storage']['company']['company_name']){
			$short_company_name = substr($form_state['storage']['company']['company_name'],0,strlen($form_state['storage']['company']['company_name'])-4);
		}
		else{
			$short_company_name = '';
		}
		$result = db_query("SELECT o.*, o.order_total-IFNULL((SELECT SUM(r.amount) FROM uc_payment_receipts r WHERE r.order_id=o.order_id),0) AS balance_due FROM uc_orders o
			INNER JOIN uc_order_products p ON o.order_id = p.order_id
			WHERE o.uid = :uid
			ORDER BY o.order_id DESC", array(':uid'=>$uid))->fetchAll(); 
		foreach($result as $rowOrder) {
			$form_state['storage']['orders'][$rowOrder->order_id] = $rowOrder;
		}
		
		// Load Renewals
		$form_state['storage']['renewals'] = array();
		$result = db_query("SELECT r.*, IF(payment_date IS NULL,total_due,0) AS balance_due FROM wra_renewals r WHERE r.uid=:uid ORDER BY r.renewal_id DESC", array(':uid'=>$uid))->fetchAll(); 
		foreach($result as $rowRenewal) {
			$form_state['storage']['renewals'][$rowRenewal->renewal_id] = $rowRenewal;
		}
		
		// Load Mail Forwarding
		$form_state['storage']['mail_forwarding'] = array();
		$result = db_query("SELECT shipment_id, pieces, weight_oz, postage, tracking_no, sent
		FROM wra_user_shipment
		WHERE to_uid=:uid AND sent > UNIX_TIMESTAMP()-31556926 ORDER BY sent DESC", array(':uid'=>$uid))->fetchAll();
		foreach($result as $rowMail) {
			$form_state['storage']['mail_forwarding'][$rowMail->shipment_id] = $rowMail;
		}
		
		// Load Notes
		$form_state['storage']['notes'] = array();
		$result = db_query('SELECT n.*,u.name as user_name FROM wra_user_notes n INNER JOIN users u ON u.uid=n.created_uid WHERE n.uid=:uid ORDER BY created DESC', array(':uid'=>$uid))->fetchAll();
		foreach($result as $rowNote) {
			$form_state['storage']['notes'][$rowNote->wra_user_note_id] = $rowNote;
		}
	//Username
	$form['name'] = array(
		'#title' => t('Username'),
		'#type' => 'textfield',
		'#size' => 30,
		'#maxlength' => 60,
		'#disabled' => $mode == 'browse' ? TRUE : FALSE
	);
		$form['name']['#value'] = isset($form_state['values']['name']);
		$form['name']['#description'] = l('Edit User','user/'.$uid.'/edit');
	$form['mail'] = array(
		'#title' => t('Email'),
		'#type' => 'textfield',
		'#size' => 60,
		'#maxlength' => 255,
		'#disabled' => $mode == 'browse' ? TRUE : FALSE
	);
		$form['mail']['#value'] = isset($form_state['values']['mail'])?$form_state['values']['mail']:'';
		$form['mail']['#description'] = l('Send Email','mailto:'.$form_state['values']['mail']);
	// Companies
	$form['companies'] = array(
		'#title' => t("Companies"),
		'#prefix' => '<div id="companies">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['company']) ? FALSE : TRUE
	);
	$companyHeader = array(t('State Filing ID'),t('Company Name'),t('Filing Date'),t('Status'),t('Options'));
	$companyRow = array();
	
	foreach($form_state['storage']['company'] as $key=>$value) {
		$eFNum = isset($value->eFNum)?$value->eFNum:'';
		$cmpid = isset($value->company_id)?$value->company_id:'';
		$companyRow[]  = array(
				
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => isset($value->filing_num)?$value->filing_num:''),
				array('data' => isset($value->company_name)?$value->company_name:''),
				array('data' => isset($value->filing_date)?$value->filing_date:''),
				array('data' => isset($value->sos_status)?$value->sos_status:''),
				array('data' => '<font size=-3>'.l('SoS','https://wyobiz.wy.gov/Business/FilingDetails.aspx?eFNum='.$eFNum).' '.
					l('View','wysos/company/browse/'.$cmpid).'</font>')
				));
	}
	$companyRow[]  = array(
		'style' => 'background-color: LightGrey;',
		'data'  => 	array(
			array('data' => l('Add Company', 'wysos/user/company/add/'.$uid )),
	));
	$form['companies']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$companyHeader, 'rows'=>$companyRow)));
	// Addresses
	$form['addresses'] = array(
		'#title' => t("Addresses"),
		'#prefix' => '<div id="addresses">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['address']) ? FALSE : TRUE
	);
	$addressHeader = array(t('First Name'),t('Last Name'),t('Address'),t('City'),t('State'),'Zip','Country','Phone','Options');
	$addressRow = array();
	
	
	foreach($form_state['storage']['address'] as $key=>$value) {
	
	//echo '<pre>';
	//print_r($value);
	//die();
	
		$street_address = isset($value->street1)?$value->street1:'';
		if(!empty($value->street2))
			$street_address .= "\r\n".$value->street2;
		$addressRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => isset($value->first_name)?$value->first_name:''),
				array('data' => isset($value->last_name)?$value->last_name:''),
				array('data' => isset($street_address)?$street_address:''),
				array('data' => isset($value->city)?$value->city:''),
				array('data' => isset($value->state)?$value->state:''),
				array('data' => isset($value->postal_code)?$value->postal_code:''),
				array('data' => isset($value->country_name)?$value->country_name:''),
				array('data' => isset($value->phone)?$value->phone:''),
				array('data' => '<font size=-3>'.l('Delete','user/'.$uid.'/addresses/'.$value->aid.'/delete').' '.l('Edit','user/'.$uid.'/addresses/'.$value->aid.'/edit').'</font>')));
	}
	
	$addressRow[]  = array(
		'style' => 'background-color: LightGrey;',
		'data'  => 	array(
			array('data' => l('Add Address', 'user/'.$uid.'/addresses/add' )),
	));
	$form['addresses']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$addressHeader, 'rows'=>$addressRow)));
	
	//  Orders
	$form['orders'] = array(
		'#title' => t("Orders"),
		'#prefix' => '<div id="orders">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
//		'#collapsed' => TRUE
		'#collapsed' => count($form_state['storage']['orders']) ? FALSE : TRUE
	);
	$orderHeader = array(t('Order No.'),t('Date'),t('Bill To'),t('Ship To'),t('Order Status'),t('Total'),t('Balance Due'));
	$orderRow = array();
	//echo '<pre>';
	//print_r($form_state['storage']['orders']);
	//die();
	
	foreach($form_state['storage']['orders'] as $key=>$value) {
		$bill_to = '';
		if(!empty($value->billing_first_name) OR !empty($value->billing_last_name))
		{
			$bill_to = substr($value->billing_first_name,0,1).' '.$value->billing_last_name.'/';
		}
		$bill_to .= substr($value->billing_company,0,20);
		$ship_to = '';
		if(!empty($value->delivery_first_name) OR !empty($value->delivery_last_name))
		{
			$ship_to = substr($value->delivery_first_name,0,1).' '.$value->delivery_last_name.'/';
		}
		$ship_to .= substr($value->delivery_company,0,20);
		$orderRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => l($value->order_id, 'admin/store/orders/'.$value->order_id)),
				array('data' => date('M-j-Y',$value->created)),
				array('data' => $bill_to),
				array('data' => $ship_to),
				array('data' => $value->order_status),
				array('data' => uc_currency_format($value->order_total)),
				array('data' => uc_currency_format($value->balance_due))
				));
	};
	
	$orderRow[]  = array(
		'style' => 'background-color: LightGrey;',
		'data'  => 	array(
			array('data' => l('New Order','wysos/user/new_order/'.$uid)),
			array('data' => ''),
			array('data' => ''),
			array('data' => ''),
			array('data' => ''),
			array('data' => ''),
			array('data' => '')
			));
	$form['orders']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$orderHeader, 'rows'=>$orderRow)));
	// Renewals
	$form['renewals'] = array(
		'#title' => t("Renewals"),
		'#prefix' => '<div id="renewals">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['renewals']) ? FALSE : TRUE
	);
	$renewalHeader = array(t('Invoice No.'),t('Due Date'),t('Filing ID'),t('Company'),t('Total'),t('Balance Due'),t('Options'));
	$renewalRow = array();
	foreach($form_state['storage']['renewals'] as $key=>$value) {
		if($value->balance_due > 0) {
			$options = '<font size=-3>'.l('Pay_CC','renew/'.$value->invoice_no).' '.l('Pay_Check','wysos/accounting/payment/'.$value->invoice_no).' ';
		} else {
			if(!empty($value->order_id))
				$options = '<font size=-3>'.l('View Order','admin/store/orders/'.$value->order_id).'</font>';
		}
		$renewalRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => $value->balance_due ? l($value->invoice_no, 'renew/'.$value->invoice_no) : $value->invoice_no),
				array('data' => $value->due_date),
				array('data' => $value->FILING_NUM),
				array('data' => $value->filing_name),
				array('data' => uc_currency_format($value->total_due)),
				array('data' => uc_currency_format($value->balance_due)),
				array('data' => isset($options)?$options:''),
				));
	}
	$form['renewals']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$renewalHeader, 'rows'=>$renewalRow)));

	// Suite
	$form['suite'] = array(
		'#title' => t("Suites"),
		'#prefix' => '<div id="suites">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['suite']) ? FALSE : TRUE
	);
	$suiteHeader = array(t('Address'),t('Mail Action'),t('Price'),t('Deposit'),t('Started'),t('Status'),t('Orig Order'),t('Recur Order'),t('Options'));
	$suiteRow = array();
	
	foreach($form_state['storage']['suite'] as $key=>$value) {
		$suiteRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => $value->street_address.' #'.$value->suite_no),
				array('data' => $value->mail_status),
				array('data' => uc_currency_format($value->price).'/'.$value->billing_period),
				array('data' => $value->deposit),
				array('data' => date('m/d/Y',$value->created)),
				array('data' => $value->status),
				array('data' => l($value->orig_order_id,'admin/store/orders/'.$value->orig_order_id)),
				array('data' => l($value->recur_order_id,'admin/store/orders/'.$value->recur_order_id)),
				array('data' => '<font size=-3>'.l('View','wysos/suite/browse/'.$value->suite_id.'</font>'))
		));
	}
	$form['suite']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header' => $suiteHeader, 'rows' => $suiteRow)));
	
	// Offices
	$form['offices'] = array(
		'#title' => t("Offices"),
		'#prefix' => '<div id="offices">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['office']) ? FALSE : TRUE
	);
	$officeHeader = array(t('Office No.'),t('Level'),t('Floorplan'),t('Type'),t('Month 2 Month'),'Year Lease','Current Rent','Options');
	$officeRow = array();
	
	foreach($form_state['storage']['office'] as $key=>$value) {
		$officeRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => $value->office_no),
				array('data' => $value->level),
				array('data' => $value->floorplan_office),
				array('data' => $value->type),
				array('data' => uc_currency_format($value->m2m_lease)),
				array('data' => uc_currency_format($value->year_lease)),
				array('data' => uc_currency_format($value->current_rent)),
				array('data' => '<font size=-3>'.l('Delete','').'</font>')));
	}
	$form['offices']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$officeHeader, 'rows'=>$officeRow)));
	
	// Mail Forwarding
	$form['mail_forwarding'] = array(
		'#title' => t("Mail Forwarding"),
		'#prefix' => '<div id="mail_forwarding">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['mail_forwarding']) ? FALSE : TRUE
	);
	$mailHeader = array(t('Date'),t('Pieces'),t('Weight'),t('Postage'),'Cost Code','Options');
	$mailRow = array();
	
	foreach($form_state['storage']['mail_forwarding'] as $key=>$value) {
		$options = '<font size=-3>';
		$options .= l('Shipment','');
		if(!empty($value->order_id))
			$options .= l($value->order_id, 'admin/store/orders/'.$value->order_id);
		$options .= '</font>';
		$mailRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => date('m/d/Y',$value->sent)),
				array('data' => $value->pieces),
				array('data' => $value->weight_oz),
				array('data' => uc_currency_format($value->postage)),
				array('data' => $value->cost_code),
				array('data' => $options),
		));
	}
	$form['mail_forwarding']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$mailHeader, 'rows'=>$mailRow)));

	// Notes
	$form['notes'] = array(
		'#title' => t("Notes"),
		'#prefix' => '<div id="notes">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => count($form_state['storage']['notes']) ? FALSE : TRUE
	);
	$noteHeader = array(t('Date'),t('From'),t('Category'),t('Note'));
	$noteRow = array();
	
	foreach($form_state['storage']['notes'] as $key=>$value) {
		$noteRow[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => date('m/d/Y',$value->created)),
				array('data' => $value->user_name),
				array('data' => $value->category),
				array('data' => $value->notes),
		));
	}
	$note_form['note_category'] = array(
//		'#title' => t('Category'),
        '#type' => 'select',
        '#options' => array(''=>'','Billing'=>'Billing','Mail'=>'Mail','Sales'=>'Sales','Support'=>'Support'),
        '#default_value' => '',
        '#required' => TRUE,
	);
	$note_form['note_text'] = array(
//		'#title' => t('Notes'),
        '#type' => 'textarea',
        '#cols' => '60',
		'#rows' => '3',
		'#maxlength' => 8192,
        '#required' => TRUE
	);
	$note_form['add_note'] = array (
		'#type' => 'submit',
		'#value' => t('Add Note')
	);
	$form['notes']['list'] = array('#type' => 'item', '#markup' => theme('table', array('header'=>$noteHeader, 'rows'=>$noteRow)));
	
	$form['note_category'] = array(
		'#title' => t('Note Category'),
        '#type' => 'select',
        '#options' => array(''=>'','Billing'=>'Billing','Mail'=>'Mail','Sales'=>'Sales','Support'=>'Support'),
        '#default_value' => '',
        '#required' => TRUE,
	);
	$form['note_text'] = array(
		'#title' => t('Notes'),
        '#type' => 'textarea',
        '#cols' => '60',
		'#rows' => '5',
		'#maxlength' => 8192,
        '#required' => TRUE
	);
	$form['add_note'] = array (
		'#type' => 'submit',
		'#value' => t('Add Note')
	);
	return $form;
}

function wysos_user_form_validate($form, &$form_state) {
}

function wysos_user_form_submit($form, &$form_state) {
//	drupal_set_message('Clicked: '.$form_state['clicked_button']['#id']);
	switch($form_state['clicked_button']['#id'])
	{
		case 'edit-add-note'	:
			include_once('wysos.util.inc');
			wysos_add_user_note($form_state['storage']['uid'], $form_state['values']['note_category'], $form_state['values']['note_text']);
			break;

	}
}

function wysos_user_company_add_form($form, &$form_state, $uid) {
	$user = user_load($uid);
	drupal_set_title('Add company to user '.$user->name);
	$form_state['storage']['uid'] = $uid;
	if(empty($form_state['storage']['referer'])) 
		//$form_state['storage']['referer'] = referer_uri();
//drupal_set_message('referer: '.$form_state['storage']['referer']);
	$form['company'] = array(
		'#title' => t('Company'),
        '#type' => 'textfield',
        '#size' => '50',
		'#maxlength' => '80',
        '#autocomplete_path' => 'wysos/autocomplete/all_companies',
		'#description' => t('Please enter Wyoming company name.')
	);
	$form['path'] = array(
		'#title' => t('Directory'),
        '#type' => 'textfield',
        '#size' => '40',
        '#required' => FALSE,
		'#description' => t('Please enter directory for documents.')
	);
	$form['ra_price'] = array(
		'#title' => t('Registered Agent Price'),
        '#type' => 'textfield',
        '#size' => '11',
        '#default_value' => '99.00',
        '#required' => TRUE,
		'#description' => t('Please enter registered agent annual fee.')
	);
	$form['nominee_sku'] = array(
		'#title' => t('Nominee Product'),
		'#type' => 'select',
		'#options' => array('' => '(None)','NPR' => 'Public Record', 'NFS' => 'Full Service'),
		'#default_value' => '',
	);
	$form['nominee_price'] = array(
		'#title' => t('Nominee Price'),
        '#type' => 'textfield',
        '#size' => '11',
		'#maxlength' => '11',
        '#required' => FALSE,
		'#description' => t('Please enter Nominee price.')
	);
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => t('Add Company')
	);
	return $form;
}

function wysos_user_company_add_form_validate($form, &$form_state) {
	// Check if Valid Company
	if(!db_query("SELECT COUNT(*) FROM wra_search_all_companies WHERE company_name=:company_name", array(':company_name'=>$form_state['values']['company']))->fetchField())
	{
		form_set_error('company','Company does not exist.');
	}
}

function wysos_user_company_add_form_submit($form, &$form_state) {
	$result = db_query("UPDATE wra_companies SET uid=:uid, ra_price=:ra_price, path=:path, nominee_sku=:nominee_sku, nominee_price=:nominee_price  
		WHERE company_id=(SELECT company_id FROM wra_search_all_companies WHERE company_name=:company_name)", 
		array(':uid'=>$form_state['storage']['uid'], ':ra_price'=>$form_state['values']['ra_price'], ':path'=>$form_state['values']['path'], ':nominee_sku'=> $form_state['values']['nominee_sku'],
		':nominee_price'=> $form_state['values']['nominee_price'],':company_name' => $form_state['values']['company']));
	//echo '<pre>';
	//print_r($result);
	//die();
	drupal_goto('wysos/user/browse/'.$form_state['storage']['uid']);
}

function wysos_user_new_order($form,&$form_state,$for_uid = NULL) {
	$address = db_query('SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
			FROM users
			INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
			INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
			WHERE users.uid = :uid',array(':uid'=>$for_uid))->fetchObject();
			//echo '<pre>';
			//print_r($address);
			//die();
	// Get Host IP
	if (!empty($_SERVER['HTTP_CLIENT_IP']))   //check ip from share internet
	{
		$ip=$_SERVER['HTTP_CLIENT_IP'];
	}
	elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR']))   //to check ip is pass from proxy
	{
		$ip=$_SERVER['HTTP_X_FORWARDED_FOR'];
	}
	else
	{
		$ip=$_SERVER['REMOTE_ADDR'];
	}
		
	if($address)	{
	// Create an Ubercart Order
	$result = db_insert('uc_orders')
			->fields(array(
				'uid' => $for_uid,
				'order_status' => 'pending',
				'order_total' => 0 ,
				'product_count' => 0,
				'primary_email' => $address->email,
				'delivery_first_name' => $address->first_name,
				'delivery_last_name' => $address->last_name,
				'delivery_phone' => $address->phone,
				'delivery_company' => $address->company,
				'delivery_street1' => $address->street1,
				'delivery_street2' => $address->street2,
				'delivery_city' => $address->city,
				'delivery_zone' => $address->zone,
				'delivery_postal_code' => $address->postal_code,
				'delivery_country' => $address->country,
				'billing_first_name' => $address->first_name,
				'billing_last_name' => $address->last_name,
				'billing_phone' => $address->phone,
				'billing_company' => $address->company,
				'billing_street1' => $address->street1,
				'billing_street2' => $address->street2,
				'billing_city' => $address->city,
				'billing_zone' => $address->zone,
				'billing_postal_code' => $address->postal_code,
				'billing_country' => $address->country,
				'payment_method' => 'credit',
				'created' => time(),
				'host' =>'',
			))->execute();
	// Goto New Order
	drupal_goto('admin/store/orders/'.$result.'/edit');
}
}
