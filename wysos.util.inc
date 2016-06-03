<?php 
function wysos_util_create_invoice_uid($uid){

	$address = db_query("SELECT first_name, last_name, phone, company, street1, street2, city, zone, uc_zones.zone_code as state, postal_code, country AS countryid, uc_countries.country_name as country, users.mail as email 
	FROM uc_addresses
	INNER JOIN uc_addresses_defaults ON uc_addresses.aid = uc_addresses_defaults.aid
	RIGHT JOIN uc_countries ON uc_countries.country_id = uc_addresses.country
	RIGHT JOIN uc_zones ON uc_zones.zone_id = uc_addresses.zone
	INNER JOIN users ON uc_addresses_defaults.uid=users.uid
	WHERE uc_addresses_defaults.uid = :uid", array(':uid'=>$uid))->fetchObject();
	
	//$address = db_fetch_object($result);
	$result = db_insert('uc_orders')
			->fields(array(
				'uid'=> $uid,
				'order_status'=> 'pending',
				'order_total'=> 0,
				'product_count'=> 0,
				'primary_email'=> $address->email,
				'delivery_first_name'=> $address->first_name,
				'delivery_last_name'=> $address->last_name,
				'delivery_phone'=> $address->phone,
				'delivery_company'=> $address->company,
				'delivery_street1'=> $address->street1,
				'delivery_street2'=> $address->street2,
				'delivery_city'=> $address->city,
				'delivery_zone'=> $address->zone,
				'delivery_postal_code'=> $address->postal_code,
				'delivery_country'=> $address->countryid,
				'billing_first_name'=> $address->first_name,
				'billing_last_name'=> $address->last_name,
				'billing_phone'=> $address->phone,
				'billing_company'=> $address->company,
				'billing_street1'=> $address->street1,
				'billing_street2'=> $address->street2,
				'billing_city'=> $address->city,
				'billing_zone'=> $address->zone,
				'billing_postal_code'=> $address->postal_code,
				'billing_country'=> $address->countryid,
				'payment_method'=> 'credit',
				'created'=> REQUEST_TIME,
				'modified'=> REQUEST_TIME,
				'host'=>'',
			))
			->execute();

	if(!$result) {
		return;
	}
	//$order_id = db_last_insert_id('uc_orders','order_id');

	return($result); 
}

function wysos_add_user_note($uid = NULL, $category=NULL, $note=NULL)
{

//print_r($category);
//die();
	global $user; 
	$result = db_insert('wra_user_notes')
			->fields(array(
				'uid' => $uid,
				'created_uid' => $user->uid,
				'category' => $category,
				'notes' => $note,
				'created' => REQUEST_TIME,
	))->execute();
	if (!$result) {
		drupal_set_message('Error Query: '.$query);
	}  
}
// Get Filing Type Abbreviation
function getFilingTypeAbbrev($filing_type)
{
	switch($filing_type) {
		case 'Profit Corporation' :
			$output = 'CORP';
			break;
		
		case 'Limited Liability Company' :
			$output = 'LLC';
			break;
		
		case 'Trade Name' :
			$output = 'TN';
			break;
		
		case 'NonProfit Corporation' :
			$output = 'NPCORP';
			break;
		
		case 'Limited Partnership' :
			$output = 'LP';
			break;

		case 'Registered Limited Liability Partnership' :
			$output = 'RLLP';
			break;

		case 'Statutory Trust' :
			$output = 'TRUST';
			break;
	}
	
	return $output;
}
/*  
 *Email Invoice
 */
function wysos_email_order($orderid,$recipient)
{
	$order = uc_order_load($orderid);
	
	if ($order === FALSE) {
	drupal_set_message(t('Order @order_id does not exist.', array('@order_id' => $orderid)));
	drupal_goto('admin/store/orders');
	}
	
	$params = array('order' => $order);
	$sent = drupal_mail('uc_order', 'invoice', $recipient, uc_store_mail_recipient_language($recipient), $params, uc_store_email_from());
	
	if (!$sent) {
	drupal_set_message(t('E-mail failed.'));
	}
	else {
	$message = t('Invoice e-mailed to @email.', array('@email' => $recipient));
	drupal_set_message($message);
	uc_order_log_changes($orderid, array($message));
	}
}


function wysos_util_create_order_from_uid($uid)
{    //print_r($uid);
	// Start a New order for the user
	// Get Users addres
	$r_address = db_query("SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
			FROM users
			INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
			INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
			WHERE users.uid = '".$uid."'")->fetchObject();
			
			//print_r($r_address);
	//$address = db_fetch_object($r_address);

	// Create an Ubercart Order
	/*
	$result = db_query("INSERT INTO {uc_orders} 
		(uid,order_status,order_total,product_count,primary_email,delivery_first_name,delivery_last_name,delivery_phone,delivery_company,delivery_street1,"
		. "delivery_street2,delivery_city,delivery_zone,delivery_postal_code,delivery_country,"
		. "billing_first_name,billing_last_name,billing_phone,billing_company,billing_street1,billing_street2,billing_city,billing_zone,billing_postal_code,"
		. "billing_country,payment_method,created,host) "
		. "VALUES (%d,'pending',%f,%d,'%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,'%s','%s','%s','%s','%s','%s','%s',%d,'%s',%d,'credit',%d,'%s')",
		$uid, 0, 0, $r_address->email, $r_address->first_name, $r_address->last_name, $r_address->phone, $r_address->company, 
		$r_address->street1, $r_address->street2, $r_address->city,$r_address->zone,$r_address->postal_code,$r_address->country,$r_address->first_name, $r_address->last_name,
		$r_address->phone, $r_address->company, $r_address->street1, $r_address->street2, $r_address->city,$r_address->zone,$r_address->postal_code,$r_address->country,time(),ip_address());
		//$uc_order_id = db_last_insert_id('uc_orders','order_id');*/
		
		
		$result1=db_insert('uc_orders')
				->fields(array(
				'uid'=>$uid,
				'order_status'=>'pending',
				'order_total'=>'0',
				'product_count'=>'0',
				'primary_email'=>$r_address->email,
				'delivery_first_name'=>$r_address->first_name,
				'delivery_last_name'=>$r_address->last_name,
				'delivery_phone'=>$r_address->phone,
				'delivery_company'=>$r_address->company,
				'delivery_street1'=>$r_address->street1,
				'delivery_street2'=>$r_address->street2,
				'delivery_city'=>$r_address->city,
				'delivery_zone'=>$r_address->zone,
				'delivery_postal_code'=>$r_address->postal_code,
				'delivery_country'=>$r_address->country,
				'billing_first_name'=>$r_address->first_name,
				'billing_last_name'=>$r_address->last_name,
				'billing_phone'=>$r_address->phone,
				'billing_company'=>$r_address->company,
				'billing_street1'=>$r_address->street1,
				'billing_street2'=>$r_address->street2,
				'billing_city'=>$r_address->city,
				'billing_zone'=>$r_address->zone,
				'billing_postal_code'=>$r_address->postal_code,
				'billing_country'=>$r_address->country,
				'payment_method'=>'credit',
				'created'=>REQUEST_TIME,
				'host'=>ip_address(),
			))->execute();
		
	if(!$result1) {
		return;
	}
	

	return($result1); 
}
