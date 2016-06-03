<?php 
function wysos_stamps_mf_user_form($form, &$form_state){
	//Abort if not IE
	if(isset($_SERVER['HTTP_USER_AGENT']) && (strpos($_SERVER['HTTP_USER_AGENT'],'MSIE') === false))
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
		'#description' => t('Please enter user name.'),
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
	);
	return $form;
}

/*
 * Validate wysos_stamps_mf_user_form_validate
 *
 */

function wysos_stamps_mf_user_form_validate($form, &$form_state){
	if(!user_load_by_name($form_state['values']['user_name'])){
		form_set_error('user_name', t('Invalid User Name'));	
	}
}
/*
 * Submit wysos_stamps_mf_user_form_submit
 *
 */
function wysos_stamps_mf_user_form_submit($form, &$form_state){
	$user = user_load_by_name($form_state['values']['user_name']);
	//Redirect to Shipment Form
	$form_state['redirect'] = 'wysos/stamps/mf_shipment/'.$user->uid;
}

/*
 * Define form prompt for New Mail Forwarding Shipment
 *
 */
function wysos_stamps_mf_shipment_form($form, $form_state, $uid) {
	// Abort if Not IE
	/* if (isset($_SERVER['HTTP_USER_AGENT']) && 
    (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE') === false))
	{
		drupal_goto('ie_required');
        return;
	}
	*/
	$mf_user = user_load($uid);
	//profile_load_profile($mf_user);
		
	$name = db_query("SELECT a.first_name, a.last_name, a.company, a.street1, a.street2, a.city, d.zone_code AS state, e.country_name AS country, a.postal_code, c.name as user_name 
		FROM uc_addresses a 
		INNER JOIN uc_addresses_defaults b ON b.aid=a.aid AND b.uid=:uid
		INNER JOIN users c ON b.uid=c.uid
		INNER JOIN uc_zones d ON a.zone = d.zone_id
		INNER JOIN uc_countries e ON a.country = e.country_id", array(':uid'=>$uid))->fetchObject();
	//$name = db_fetch_object($r_name);
	$is_ra = db_query("SELECT COUNT(*) FROM wra_registered_agents WHERE uid=:uid", array(':uid'=>$uid))->fetchField();
	
	$mail_action = db_query("SELECT mail_status FROM wra_suites WHERE uid=:uid AND status='active' LIMIT 1", array(':uid'=>$uid))->fetchField();
	
	$num_company = db_query("SELECT COUNT(*) FROM wra_companies WHERE uid=:uid AND sos_status IN('Active','Delinquent - Tax','Delinquent - Registered Agent',
		'Delinquent - Other')", array(':uid'=>$uid))->fetchField();
		
	$totals = db_query("SELECT COUNT(*) AS shipments,SUM(pieces) AS pieces,SUM(postage) AS postage, wra_suites.status FROM `wra_mf_shipment` 
		LEFT JOIN wra_suites ON wra_mf_shipment.to_uid = wra_suites.uid AND wra_suites.status = 'active'
		WHERE to_uid=:uid AND sent > UNIX_TIMESTAMP()-31556926", array(':uid'=>$uid))->fetchObject();
		
	//$totals = db_fetch_object($r_totals);
	
	if($name){
	drupal_set_title('Shipment for '.$name->first_name.' '.$name->last_name.' ('.$name->user_name.')');}
    $service_level = '';
	if(empty($mail_action) OR $mail_action == 'mail_forward')
	{
		setlocale(LC_MONETARY, 'en_US');
		$color = '#00FF00';
		if($is_ra) {
				$service_level = 'Affiliate RA';
				$cost_code = 'Mail Forwarding';
		} else {
			if($totals->status == 'active') {
				$service_level = 'Virtual Office Paid';
				$cost_code = 'Virtual Office';
			} else {
				$service_level = 'Registered Agent';
				if($totals->pieces > 20 * $num_company) {
					$color = '#FF0000';
					$cost_code = 'Forwarding Overage';
					$create_invoice_option = TRUE;
				} else
					$cost_code = 'Mail Forwarding';
			}
		}
	
		// Table tag attributes
		$attributes = array(
		'border'      => 0,
		'cellspacing' => 0,
		'cellpadding' => 3,
		'align'       => 'left',
		'bgcolor'	  => $color
		);
		$headers = array(array('data' => t('Shipments'), 'class' => 'header-cell-class', 'header'=>TRUE),
			array('data' => t('Pieces'), 'class' => 'header-cell-class', 'header'=>TRUE),
			array('data' => t('Postage'), 'class' => 'header-cell-class', 'header'=>TRUE),
			array('data' => t('Entities'), 'class' => 'header-cell-class', 'header'=>TRUE),
			array('data' => t('Service'), 'class' => 'header-cell-class', 'header'=>TRUE));
		$row = array();
				
		$row[] = array(array('data' => $totals->shipments,'bgcolor'=>$color), array('data' => $totals->pieces,'bgcolor'=>$color), array('data' => $totals->postage,'bgcolor'=>$color),
		array('data'=>$num_company,'bgcolor'=>$color),array('data'=>isset($service_level)?$service_level:'','bgcolor'=>$color));
		
		$form['title'] = array(
			'#type' => 'item',
			'#markup' => theme('table', array('header'=>$headers, 'rows'=>$row)), 
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
			'#value' => $cost_code
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
			'#default_value' => '1',
			'#required' => TRUE
		);
		if($cost_code == 'Forwarding Overage')
		{
			$form['create_invoice'] = array(
				'#title' => t('Create Invoice'),
				'#type' => 'checkbox',
				'#default_value' => true,
			);
			$form['charge_invoice'] = array(
				'#title' => t('Charge Customer'),
				'#type' => 'checkbox',
				'#default_value' => true,
			);
		}
		$recipient = '';
		if($name){
		//if(!empty($name->company))
			$recipient .= $name->company. "\\r\\n";
		$recipient .= 	$name->first_name . ' '. $name->last_name . "\\r\\n";
		$recipient .=  $name->street1 . "\\r\\n";
		//if(!empty($name->street2))
			$recipient .= $name->street2. "\\r\\n";
		$recipient .= $name->city . ', ' . $name->state . ' ' . $name->postal_code;
		//if($name->country != 'United States')
			$recipient .= "\\r\\n" . $name->country;
	//			print_job.PrinterName = "Xerox Phaser 6180MFP-D PCL 6";
	//			print_job.MailpieceWeight = document.wysos-stamps-mf-shipment-form.edit-weight-oz.value;
	//			print_job.AddRecipient('.$recipient.');
	}
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
					print_job.ReturnAddress = "Wyoming Business Office\\r\\n2510 Warren Ave.\\r\\nCheyenne, WY 82001";
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
					print_job.ReturnAddress = "Wyoming Business Office\\r\\n2510 Warren Ave.\\r\\nCheyenne, WY 82001";
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
	} else {
		$form['mail_status'] = array(
			'#title' => 'Mail Status',
			'#type' => 'item',
			'#value' => $mail_action,
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
	}
	return $form; 
}
function wysos_stamps_mf_shipment_form_validate($form, &$form_state) {
	if($form_state['values']['weight_total'] <= 0)
		form_set_error('weight_total','Weight must be more than 0.');
	if($form_state['values']['postage'] <= 0)
		form_set_error('postage','Postage must be more than 0.');
}

function wysos_stamps_mf_shipment_form_submit($form, &$form_state) {
//drupal_set_message('form_state: '.print_r($form_state, true));
$uid = arg(3);
	$mf_user = user_load($uid);
	global $user;

	if($form_state['values']['create_invoice']) {
		// Get Host IP
		if (!empty($_SERVER['HTTP_CLIENT_IP']))   //check ip from share internet
			$ip=$_SERVER['HTTP_CLIENT_IP'];
		elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR']))   //to check ip is pass from proxy
			$ip=$_SERVER['HTTP_X_FORWARDED_FOR'];
		else
			$ip=$_SERVER['REMOTE_ADDR'];
			
		// Get overage fee
		$over_fee = db_query("SELECT list_price FROM uc_products WHERE model='MFO'")->fetchField();
		$order_total = $over_fee + $form_state['values']['postage'];
		
		// Get uc_address
		$address = db_query("SELECT uc_addresses.* FROM uc_addresses
		INNER JOIN uc_addresses_defaults ON uc_addresses.aid = uc_addresses_defaults.aid
		WHERE uc_addresses_defaults.uid=:uid", array(':uid'=>$mf_user->uid))->fetchObject();
		// Add uc_orders
		$orderid = db_insert('uc_orders')
				->fields(array(
					'uid' => $mf_user->uid,
					'order_status' => 'pending',
					'order_total' => $order_total,
					'product_count' => 1,
					'primary_email' => $mf_user->mail,
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
					'created' => REQUEST_TIME,
					'host' => '',
				))->execute();
		
		if(!$orderid) {
			return;
		}
		//$orderid = db_last_insert_id('uc_orders','order_id');
		
		// Ad Overage uc_order_products
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => '1245',
					'model' => 'MFO',
					'title' => 'Mail Forwarding Overage Fee',
					'qty' => 1,
					'price' => $over_fee,
				))->execute();
		if(!$result) {
			return;
		}
		// Ad Shipping uc_order_products
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => '1233',
					'model' => 'SHIP',
					'title' => 'Postage',
					'qty' => 1,
					'price' => $form_state['values']['postage'],
				))->execute();
		if(!$result) {
			return;
		}		
		$result = db_insert('wra_mf_shipment')
				->fields(array(
					'to_uid' => $mf_user->uid,
					'weight_oz' => $form_state['values']['weight_total'],
					'sent' => REQUEST_TIME,
					'pieces' => $form_state['values']['pieces'],
					'postage' => $form_state['values']['postage'],
					'tracking_no' => $form_state['values']['tracking'],
					'cost_code' => $form_state['values']['cost_code'],
					'created_uid' => $user->uid,
					'order_id' => $orderid,
				))->execute();		
		
		if (!$result) {
			return;
		}  

		require_once('wysos.util.inc');
		wysos_email_order($orderid, $mf_user->mail);

		setlocale(LC_MONETARY, 'en_US');
		drupal_set_message("Order ".l($orderid,'admin/store/orders/'.$orderid).' '.l('(Print)','admin/store/orders/'.$orderid.'/invoice/print').' for '.
			money_format('%n', $form_state['values']['postage'])." added.");
			
		//drupal_redirect_form($form,'wysos/stamps/mf_user');
		
		$form_state['redirect'] = 'wysos/stamps/mf_user';
		
	} else {
		$result = db_query("INSERT wra_mf_shipment (to_uid,weight_oz,sent,pieces,postage,tracking_no,cost_code,created_uid) 
			VALUES (%d,%f,UNIX_TIMESTAMP(),%d,%f,'%s','%s',%d)",
			$mf_user->uid,$form_state['values']['weight_total'],$form_state['values']['pieces'],$form_state['values']['postage'],$form_state['values']['tracking'],
			$form_state['values']['cost_code'], $user->uid);
		
		$result	= db_insert('wra_mf_shipment')
				->fields(array(
					'to_uid' => $mf_user->uid,
					'weight_oz' => $form_state['values']['weight_total'],
					'sent' => REQUEST_TIME,
					'pieces' => $form_state['values']['pieces'],
					'postage' => $form_state['values']['postage'],
					'tracking_no' => $form_state['values']['tracking'],
					'cost_code' => $form_state['values']['cost_code'],
					'created_uid' => $user->uid,
				))->execute();
		
		if (!$result) {
			return;
		}  
		
		setlocale(LC_MONETARY, 'en_US');
		drupal_set_message("Shipment for ".$form_state['values']['postage']." added.");
		//drupal_redirect_form($form,'wysos/stamps/mf_user');
		$form_state['redirect'] = 'wysos/stamps/mf_user';
	}

		
}




