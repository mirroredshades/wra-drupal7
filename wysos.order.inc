<?php 

function wysos_order_import_form($form,&$form_state){
	$form['download'] = array(
		'#type' => 'submit',
		'#id' => 'download',
		'#value' => 'Download Orders'
	);
	
	$headers = array(
		array(	
			'data' => t('Order'),
			'field' => 'OrderNumber',
			'sort' => 'DESC'),
		array(
			'data' => t('Submitted'),
			'field' => 'OrderDate'),
		array(
			'data' => t('Type'),
			'field' => 'OrderType'),
		array(
			'data' => t('Total'),
			'field' => 'Total'),
		array(
			'data' => t('Pay By'),
			'field' => 'PayMethod'),
		array(
			'data' => t('Customer'),
			'field' => 'CustomerName'),
		array(
			'data' => t('Company'),
			'field' => 'CompanyName'),
			'options'
	);
	
	// Generate table
		$query = "SELECT fed_www.orders.OrderId, fed_www.orders.OrderNumber, DATE_FORMAT(fed_www.orders.OrderDate,'%Y-%m-%%d %H:%i') AS OrderDate, fed_www.orders.OrderType,
			LEFT(CONCAT(fed_www.orders.CustomerFirstName,CONCAT(' ',fed_www.orders.CustomerLastName)),16) AS Customer, LEFT(fed_www.orders.PayMethod,5) AS PayMethod, CONCAT('$',fed_www.orders.Total) As Total, 
			LEFT(fed_www.orders.CompanyName,16) AS CompanyName, fed_www.orders.uc_orderid 
			FROM fed_www.orders 
			WHERE fed_www.orders.OrderDate>DATE_SUB(CURDATE(),INTERVAL 45 DAY) AND fed_www.orders.ProcessedDt IS NULL"; 
	include_once('wysos.util.inc');		
		$form['data'] = array(
			'#type' => 'markup',
			//'#value' => generate_table($query,20,$headers,'wysos_order_import_build_row','wysos_order_import_opts')
			);	
		return $form;
}

function wysos_order_import_form_submit($form, &$form_state) {
	// Download the web orders
	$output = shell_exec('/home/wyregnet/dl_www_orders.sh');
	drupal_set_message('Download returns: '.$output);
}
function wysos_order_import_view($orderid=NULL) {
	/*
	if($orderid == NULL) {
		// Display Recent non-imported orders
		require_once('wysos.util.inc');
		
		// Table Header
		$headers = array(
			array(
				'data' => t('Order'),
				'field' => 'OrderNumber',
				'sort' => 'desc' ),
			array(
				'data' => t('Submitted'),
				'field' => 'OrderDate'),
			array(
				'data' => t('Type'),
				'field' => 'OrderType'),
			array(
				'data' => t('Total'),
				'field' => 'phone_no'),
			array(
				'data' => t('Pay By'),
				'field' => 'PayMethod'),
			array(
				'data' => t('Customer'),
				'field' => 'CutomerName'),
			array(
				'data' => t('Company'),
				'field' => 'CompanyName'),
				'options'
			);
	
		// Generate table
		$query = "SELECT fed_www.orders.OrderId, fed_www.orders.OrderNumber, DATE_FORMAT(fed_www.orders.OrderDate,'%Y-%m-%d %H:%i') AS OrderDate, fed_www.orders.OrderType,
			LEFT(CONCAT(fed_www.orders.CustomerFirstName,CONCAT(' ',fed_www.orders.CustomerLastName)),16) AS Customer, LEFT(fed_www.orders.PayMethod,5) AS PayMethod, CONCAT('$',fed_www.orders.Total) As Total, 
			LEFT(fed_www.orders.CompanyName,16) AS CompanyName, fed_www.orders.uc_orderid 
			FROM fed_www.orders 
			WHERE fed_www.orders.OrderDate>DATE_SUB(CURDATE(),INTERVAL 251 DAY) AND fed_www.orders.ProcessedDt IS NULL";
		include_once('wysos.util.inc');		
	  return generate_table($query,20,$headers,'wysos_order_import_build_row','wysos_order_import_opts');
	} else {
		// Display Specific Order
		$weborder = db_query("SELECT OrderNumber,OrderType,DATE_FORMAT(fed_www.orders.OrderDate,'%Y-%m-%%d %H:%i') AS OrderDate, Total,CompanyName,CustomerFirstName,CustomerLastName,LEFT(CONCAT(fed_www.orders.CustomerFirstName,CONCAT(' ',fed_www.orders.CustomerLastName)),16) AS Customer,CustomerAddress,CustomerCity,CustomerState,CustomerZip,CustomerCountry,CustomerPhone,CustomerEmail,
			Comments,FromIP,FD_OrderNum,ApprovalDt,LOWER(LEFT(PayMethod,2)) AS PayMethod, uc_orderid
			FROM fed_www.orders WHERE OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
		//$weborder = db_fetch_array($result);
		
		$row[] = array();
		$row[] = array(
			'style' => 'background-color: LightGrey;',
			'data'  => array(array(
				'data' => 'Web Order:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->OrderNumber,
				'style' => $color),
			array('data' => 'Date:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->OrderDate,
				'style' => $color)));
		$row[] = array(
			'style' => 'background-color: LightGrey;',
			'data'  => array(array(
				'data' => 'Company:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->CompanyName,
				'style' => $color),
			array('data' => 'Type:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->OrderType,
				'style' => $color)));
		$row[] = array(
			'style' => 'background-color: LightGrey;',
			'data'  => array(array(
				'data' => 'Customer:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->Customer,
				'style' => $color),
			array('data' => 'Total:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->Total,
				'style' => $color)));
		$row[] = array(
			'style' => 'background-color: LightGrey;',
			'data'  => array(array(
				'data' => 'Comments:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => $weborder->Comments,
				'colspan' => 3)));
			

		switch($weborder->OrderType) {
			case 'LLC' :
				// Get the LLC order
				$llcorder = db_query("SELECT * FROM {fed_www.llcorders} WHERE OrderId=:orderid", array(':orderid'=>$weborderid));
				//$llcorder = db_fetch_array($result3);
				// create data parameter
				break;
	
			case 'PROFITCORP' :
				// Get the profitcorp order
				$profitcorporder = db_query("SELECT * FROM {fed_www.profitcorporders} WHERE OrderId=:orderid", array(':orderid'=>$weborderid));
				//$profitcorporder = db_fetch_array($result3);
				break;
	
			case 'CHANGERA' :
				// create data parameter
				break;
	
			case 'VOFFICE' :
				break;
	
			case 'LP' :
				break;
	
			case 'NOMINEE' :
				break;
		}
	}
	
	//$output = theme('table', $header, $row, $attributes);
	$output = theme('table', array('header'=>$header, 'rows'=>$row, 'attributes'=>$attributes));
	*/
	$output = array();
	return $output;
}

/*123*/

function wysos_order_import($weborderid = null) {
	if (!is_numeric($weborderid)) {
	// We will just show a standard "access denied" page in this case.
	return drupal_access_denied();
	} 
	// Load the Order 
	$weborder = db_query("SELECT OrderNumber,OrderType,Total,CompanyName,CustomerFirstName,CustomerLastName,CustomerAddress,CustomerCity,CustomerState,CustomerZip,CustomerCountry,CustomerPhone,CustomerEmail,
		Comments,FromIP,FD_OrderNum,ApprovalDt,LOWER(LEFT(PayMethod,2)) AS PayMethod, uc_orderid
		FROM fed_www.orders WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid));
	//$weborder = db_fetch_array($result);
	
	// Get UID
	$uid = db_query("SELECT uid FROM users WHERE mail=:mail", array(':mail'=>$weborder->CustomerEmail))->fetchField();
	if($uid == FALSE) {
		// Add new user
		$uname = strtolower(substr(trim($weborder->CustomerFirstName), 0, 1).trim($weborder->CustomerLastName));
		$baseuname = $uname;
		// check if uname is taken
		if(db_query("SELECT COUNT(name) FROM {users} WHERE name=:name", array(':name'=>$uname))->fetchField() > 0)
		{
			$uname = $baseuname . mt_rand(0,999);
		}
		$newuser = array(
			'name' => $uname,
			'pass' => $uname.'11',
			'mail' => $weborder->CustomerEmail,
			'status' => 1,
			'init' => $weborder->CustomerEmail,
			'created' => REQUEST_TIME,
		);
		
		$nuser = user_save(NULL,$newuser);
		// Add Customer Role to user
			
		user_multiple_role_edit(array($nuser->uid), 'add_role', 4);	
		// Add Address
		
		$countryid = db_query("SELECT country_id FROM {uc_countries} WHERE country_iso_code_2=:cid", array('cid'=>$weborder->CustomerCountry));
		$zoneid = db_query("SELECT zone_id FROM {uc_zones} WHERE zone_country_id=:zci AND (zone_name=:zn OR zone_code=:zc)", array(':zci'=>$countryid, ':zn'=>$weborder->CustomerState, ':zc'=>$weborder->CustomerState));
		$aid = db_insert('uc_addresses')
					->fields(array(
						'uid' => $nuser->uid,
						'first_name' => $weborder->CustomerFirstName,
						'last_name' => $weborder->CustomerLastName,
						'phone' => $weborder->CustomerPhone,
						'company' => $weborder->CompanyName,
						'street1' => $weborder->CustomerAddress,
						'city' => $weborder->CustomerCity,
						'zone' => $zoneid,
						'postal_code' => $weborder->CustomerZip,
						'country' => $countryid,
						'address_name' => 'Home',
						'created' => REQUEST_TIME,
						'modified' => REQUEST_TIME,
					))->execute();
		$uid = 	$nuser->uid;
		$uad = db_insert('uc_addresses_defaults')
				->fields(array(
					'aid' => $aid,
					'uid' => $nuser->uid,				
				))->execute();			
		// Set As Default Address
		//$result = db_query("INSERT INTO uc_addresses_defaults (aid, uid) VALUES (%d,%d)", $aid, $uid);
	}
	$order_user = user_load($uid);
	
	// Load User's Default Address
	$address = db_query("SELECT first_name, last_name, phone, company, street1, street2, city, zone, uc_zones.zone_code as state, postal_code, country AS countryid, uc_countries.country_name as country 
	FROM uc_addresses
	INNER JOIN uc_addresses_defaults ON uc_addresses.aid = uc_addresses_defaults.aid
	INNER JOIN uc_countries ON uc_countries.country_id = uc_addresses.country
	INNER JOIN uc_zones ON uc_zones.zone_id = uc_addresses.zone
	WHERE uc_addresses_defaults.uid = :uid", array(':uid'=>$uid))->fetchObject();
	
	//$address = db_fetch_array($result);
//drupal_set_message('Order: '.$weborderid.' UID: '.$uid);
//drupal_set_message('WebOrder: '.print_r($weborder, true));

	// Get Total, NumProducts
	switch($weborder->OrderType) {
		case 'LLC' :
			// Get the LLC order
			$price = 199;
			$llcorder = db_query("SELECT * FROM {fed_www.llcorders} WHERE OrderId=:weborderid", array(':weborderid'=>$weborderid))->fetchObject();
			//$llcorder = db_fetch_array($result3);
			// create data parameter
			$attributes['Name of the company'] = array($weborder->CompanyName);
			if ($llcorder->NomineeType == 'None') {
				$attributes['Nominee'] = array('(none)');
				$attributes['Manager #1 Name'] = array($llcorder->Manager1Name);
				$attributes['Manager #1 Address'] = array($llcorder->Manager1Address.','.$llcorder->Manager1City.','.$llcorder->Manager1State.' '.$llcorder->Manager1Zip.','.$llcorder->Manager1Country);
				if(!empty($llcorder->Manager2Name)) {
					$attributes['Manager #2 Name'] = array($llcorder->Manager2Name);
					$attributes['Manager #2 Address'] = array($llcorder->Manager2Address.','.$llcorder->Manager2City.','.$llcorder->Manager2State.' '.$llcorder->Manager2Zip.','.$llcorder->Manager2Country);
				}
				if(!empty($llcorder->Manager3Name)) {
					$attributes['Manager #3 Name'] = array($llcorder->Manager3Name);
					$attributes['Manager #3 Address'] = array($llcorder->Manager3Address.','.$llcorder->Manager3City.','.$llcorder->Manager3State.' '.$llcorder->Manager3Zip.','.$llcorder->Manager3Country);
				}
			} else {
				$attributes['Manager #1 Name'] = array($weborder->CustomerFirstName.' '.$weborder->CustomerLastName);
				$attributes['Manager #1 Address'] = array($llcorder->CustomerAddress.','.$llcorder->CustomerCity.','.$llcorder->CustomerState.' '.$llcorder->CustomerZip.','.$llcorder->CustomerCountry);
			}

			$options = array();
			if($llcorde->EIN) {
				$options[] = 'EIN';
				$price += 75;
			}
			if($llcorder->RecordBook) {
				$options[] = 'Corporate Record Book, Seal, Stock Certificates and other Forms';
				$price += 99;
			}
			if($llcorder->AttyOpAgreement) {
				$options[] = 'Attorney Prepared Operating Agreement and Initial Meeting';
				$price += 350;
			}
			if(count($options) > 0) {
				$attributes['Options'] = $options;	
			} 
			
			
			switch($llcorder->NomineeType) {
				case 'PublicRecord' :
					$attributes['Nominee'] = array('Public Record');
					$price+=199;
					break;
					
				case 'FullService' :
					$attributes['Nominee'] = array('Full Service');
					$price+=595;
					break;
					
				case 'Attorney' :
					$attributes['Nominee'] = array('Attorney');
					$price+=1495;
					break;
			}

//			wysos_order_import_order_options($llcorder,$order_product,$total);
//			wysos_order_import_nominee_product($llcorder->NomineeType,$order_product,$total);
			if(db_query("SELECT COUNT(*) FROM {fed_www.vofficeorders} WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid))->fetchField()) {
				$vofficeorder = db_query("SELECT CompanyName,LEFT(VirtualOfficeTerm,9) AS VirtualOfficeTerm,ItemPrice FROM {fed_www.vofficeorders} WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid))->fetchObject();
				//$vofficeorder = db_fetch_array($result);
			
				switch($vofficeorder->VirtualOfficeTerm) {
					case '$49 Month' :
						$attributes['Virtual Office Term'] = 'Monthly';
						$price+=49;
						break;
						
					case '$499 Year' :
						$attributes['Virtual Office Term'] = 'Annually';
						$price+=499;
						break;
						
					default:
						$attributes['Virtual Office Term'] = '(none)';
						break;
				}
			}

			$order_product = array();
			$order_product[0]['data'] = array("attributes"=>$attributes);
			$order_product[0]['nid'] = 2;
			$order_product[0]['model'] = 'WYLLC1R';
			$order_product[0]['title'] = 'Wyoming LLC + 1 Year Registered Agent';
			$order_product[0]['price'] = $price;
			$total = $price;
			break;

		case 'PROFITCORP' :
			// Get the profitcorp order
			$price = 199;
			$profitcorporder = db_query("SELECT * FROM {fed_www.profitcorporders} WHERE OrderId=:weborderid", array(':weborderid'=>$weborderid))->fetchObject();
			//$profitcorporder = db_fetch_array($result3);
			// create data parameter
			$attributes['Name of the company'] = array($weborder->CompanyName);
			if ($profitcorporder->NomineeType == 'None') {
				$attributes['Nominee'] = array('(none)');
				$attributes['Director #1 Name'] = array($profitcorporder->PresidentName);
				$attributes['Director #1 Address'] = array($profitcorporder->PresidentAddress.','.$profitcorporder->PresidentCity.','.$profitcorporder->PresidentState.
				' '.$profitcorporder->PresidentZip.','.$profitcorporder->PresidentCountry);

				if(!empty($profitcorporder->VPName)) {
					$attributes['Director #2 Name'] = array($profitcorporder->VPName);
					$attributes['Director #2 Address'] = array($profitcorporder->VPAddress.','.$profitcorporder->VPCity.','.$profitcorporder->VPState.
					' '.$profitcorporder->VPZip.','.$profitcorporder->VPCountry);
				}
				if(!empty($profitcorporder->SecretaryName)) {
					$attributes['Director #3 Name'] = array($profitcorporder->SecretaryName);
					$attributes['Director #3 Address'] = array($profitcorporder->SecretaryAddress.','.$profitcorporder->SecretaryCity.','.$profitcorporder->SecretaryState.
					' '.$profitcorporder->SecretaryZip.','.$profitcorporder->SecretaryCountry);
				}
			} else {
				// Nominee Active
				$attributes['Director #1 Name'] = array($weborder->CustomerFirstName.' '.$weborder->CustomerLastName);
				$attributes['Director #1 Address'] = array($weborder->CustomerAddress.','.$weborder->CustomerCity.','.$weborder->CustomerState.
				' '.$weborder->CustomerZip.','.$weborder->CustomerCountry);
			}
			// Add Shares and Class
			$attributes['Authorized Shares'] = $profitcorporder->NumShares;
			$attributes['Share Class'] = $profitcorporder->ShareClass;
			$attributes['Par Value'] = $profitcorporder->SharePar;
			$options = array();
			if($profitcorporder->EIN) {
				$options[] = 'EIN';
				$price += 75;
			}
			if($profitcorporder->RecordBook) {
				$options[] = 'Corporate Record Book, Seal, Stock Certificates and other Forms';
				$price += 99;
			}
			if($profitcorporder->AttyOpAgreement) {
				$options[] = 'Attorney Prepared Operating Agreement and Initial Meeting';
				$price += 350;
			}
			if(count($options) > 0) {
				$attributes['Options'] = $options;	
			}
			
			switch($profitcorporder->NomineeType) {
				case 'PublicRecord' :
					$attributes['Nominee'] = array('Public Record');
					$price+=199;
					break;
					
				case 'FullService' :
					$attributes['Nominee'] = array('Full Service');
					$price+=595;
					break;
					
				case 'Attorney' :
					$attributes['Nominee'] = array('Attorney');
					$price+=1495;
					break;
			}

			if(db_query("SELECT COUNT(*) FROM {fed_www.vofficeorders} WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid))->fetchField()) {
				$vofficeorder = db_query("SELECT CompanyName,LEFT(VirtualOfficeTerm,9) AS VirtualOfficeTerm,ItemPrice FROM {fed_www.vofficeorders} WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid))->fetchObject();
				//$vofficeorder = db_fetch_array($result);
			
				switch($vofficeorder->VirtualOfficeTerm) {
					case '$49 Month' :
						$attributes['Virtual Office Term'] = 'Monthly';
						$price+=49;
						break;
						
					case '$499 Year' :
						$attributes['Virtual Office Term'] = 'Annually';
						$price+=499;
						break;
						
					default:
						$attributes['Virtual Office Term'] = '(none)';
						break;
				}
			}
			$order_product = array();
			$order_product[0]['data'] = array("attributes"=>$attributes);
			$order_product[0]['nid'] = 1;
			$order_product[0]['model'] = 'WYP1R';
			$order_product[0]['title'] = 'Profit Corporation + 1 Year Registered Agent';
			$order_product[0]['price'] = $price;
			$total = $price;
			break;

		case 'CHANGERA' :
			// create data parameter
			$attributes['Name of the company'] = array($weborder->CompanyName);
			$options = array();
			
			$order_product = array();
			$order_product[0]['data'] = array("attributes"=>$attributes);
			$order_product[0]['nid'] = 6;
			$order_product[0]['model'] = 'WY1R';
			$order_product[0]['title'] = 'Wyoming Registered Agent Service - 1 Year';
			$order_product[0]['price'] = $weborder->Total;
			$total = $weborder->Total;
			break;

		case 'VOFFICE' :
			$total = 0;
			$order_product = array();
			wysos_order_import_voffice_product($weborderid,$order_product,$total,$weborder->CompanyName);
			break;

		case 'LP' :
			break;

		case 'NOMINEE' :
			$total = 0;
			$order_product = array();
			// Get the nominee order
			$nomineeorder = db_query("SELECT * FROM {fed_www.nomineeorders} WHERE OrderId=:weborderid", array(':weborderid'=>$weborderid))->fetchObject();
			//$nomineeorder = db_fetch_array($result3);

			wysos_order_import_nominee_product($nomineeorder->NomineeType,$order_product,$total);
			break;
	}
	// Set payment_method
	switch($weborder->PayMethod) {
		case 'am'	:
		case 'vi'	:
		case 'mc'	:
		case 'di'	:
			$pay_method = 'credit';
			$order_status = 'payment_received';
			break;
			
		case 'ch'	:
		case 'ba'	:
		case 'ma'	:
			$pay_method = 'check';
			$order_status = 'pending';
	};
	
	$countryid = db_query("SELECT country_id FROM {uc_countries} WHERE country_iso_code_2=:cc", array(':cc'=>$weborder->CustomerCountry))->fetchField();
	$zoneid = db_query("SELECT zone_id FROM {uc_zones} WHERE zone_country_id=:zci AND (zone_name=:zn OR zone_code=:zc)", array(':zci'=>$countryid, ':zn'=>$weborder->CustomerState, ':zc'=>$weborder->CustomerState))->fetchField();
	
	$orderid = db_insert('uc_orders')
			->fields(array(
				'uid' => $uid,
				'order_status'=> $order_status,
				'order_total'=> $weborder->Total,
				'product_count'=> count($order_product),
				'primary_email'=> $order_user->mail,
				'delivery_first_name'=> $weborder->CustomerFirstName,
				'delivery_last_name'=> $weborder->CustomerLastName,
				'delivery_phone'=> $weborder->CustomerPhone,
				'delivery_company'=> $weborder->CompanyName,
				'delivery_street1'=> $weborder->CustomerAddress,
				'delivery_street2'=> '',
				'delivery_city'=> $weborder->CustomerCity,
				'delivery_zone'=> $zoneid,
				'delivery_postal_code'=> $weborder->CustomerZip,
				'delivery_country'=> $countryid,
				'billing_first_name'=> $weborder->CustomerFirstName,
				'billing_last_name'=> $weborder->CustomerLastName,
				'billing_phone'=> $weborder->CustomerPhone,
				'billing_company'=> $weborder->CompanyName,
				'billing_street1'=> $weborder->CustomerAddress,
				'billing_street2'=> '',
				'billing_city'=> $weborder->CustomerCity,
				'billing_zone'=> $zoneid,
				'billing_postal_code'=> $weborder->CustomerZip,
				'billing_country'=> $countryid,
				'payment_method'=> $pay_method,
				'created'=> REQUEST_TIME,
				'host'=> $weborder->FromIP,
			))->execute();
	
	if(!$orderid) {
		return;
	}
	//$orderid = db_last_insert_id('uc_orders','order_id');
	
	// Ad uc_order_products
	foreach ($order_product as $value) {

		$result = db_insert('uc_order_products')
			->fields(array(
				'order_id' => $orderid,
				'nid' => $value['nid'],
				'model' => $value['model'],
				'title' => $value['title'],
				'qty' => 1,
				'price' => $value['price'],
				'data' => serialize($value['data']),
			))->execute();
			
		
		if(!$result) {
			return;
		}
	}
	
	// Add Payment Receipt if paid by credit card
	if($pay_method == 'credit' && !empty($weborder->FD_OrderNum)) {
		// Add uc_payment_receipt
		$comment = "linkpoint: " . $weborder->FD_OrderNum;
				
		$result = db_insert('uc_payment_receipts')
			->fields(array(
				'order_id' => $orderid,
				'method' => 'Credit card',
				'amount' => $weborder->Total,
				'uid' => $uid,
				'comment' => $comment,
				'received' => REQUEST_TIME,
			))->execute();		
				
		if (!$result) {
			return;
		}
	
	}
	
	// Update Status and uc_orderid on www.wyomingregisteredagent.com
	
	$result = db_update('fed_www.orders')
			->fields(array(
				'Status' => 'Imported: my. Order '.$weborder->OrderNumber,
				'uc_orderid' => $orderid,
			))->condition('OrderId',$weborderid)->execute();
					
	// Add comment to Ubercart order
	uc_order_comment_save($orderid, $uid, 'Imported from www.wyomingregisteredagent.com order '.$weborder->OrderNumber,'order');
	
	drupal_set_message('www.wyomingregisteredagent.com order '.$weborder->OrderNumber.' imported as Order '.l($orderid,'admin/store/orders/'.$orderid));
	drupal_goto('wysos/order_import');
}
/* 
 * Add Virtual Office Product to array
*/
function wysos_order_import_voffice_product($weborderid,&$order_product,&$total,$companyname) {
	if( db_query("SELECT COUNT(*) FROM {fed_www.vofficeorders} WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid))->fetchField() == 0) {
		return;
	}
	$vofficeorder = db_query("SELECT CompanyName,LEFT(VirtualOfficeTerm,9) AS VirtualOfficeTerm,ItemPrice FROM {fed_www.vofficeorders} WHERE OrderId=:weborderid",array(':weborderid'=>$weborderid))->fetchObject();
	//$vofficeorder = db_fetch_array($result);
	$counter = count($order_product);

	switch($vofficeorder->VirtualOfficeTerm) {
		case '$49 Month' :
			$attributes['Name of the company'] = array($companyname);
			$order_product[$counter]['data'] = array("attributes"=>$attributes);
			$order_product[$counter]['nid'] = 4;
			$order_product[$counter]['model'] = 'WYVO1M';
			$order_product[$counter]['title'] = 'Wyoming Virtual Office - Monthly';
			$order_product[$counter]['price'] = $vofficeorder->ItemPrice;
			$total+=$vofficeorder->ItemPrice;
			break;
			
		case '$499 Year' :
			$attributes['Name of the company'] = array($companyname);
			$order_product[$counter]['data'] = array("attributes"=>$attributes);
			$order_product[$counter]['nid'] = 3;
			$order_product[$counter]['model'] = 'WYVO1Y';
			$order_product[$counter]['title'] = 'Wyoming Virtual Office - Annually';
			$order_product[$counter]['price'] = $vofficeorder->ItemPrice;
			$total+=$vofficeorder->ItemPrice;
			break;
	}
}

/* 
 * Add Nominee Product to array
*/
function wysos_order_import_nominee_product($nomineetype,&$order_product,&$total) {
	$counter = count($order_product);
	switch($nomineetype) {
		case 'PublicRecord' :
			$order_product[$counter]['nid'] = 5;
			$order_product[$counter]['model'] = 'NPR';
			$order_product[$counter]['title'] = 'Public Record Nominee Officer';
			$order_product[$counter]['price'] = 199;
			$total+=199;
			break;
			
		case 'FullService' :
			$order_product[$counter]['nid'] = 1096;
			$order_product[$counter]['model'] = 'NFS';
			$order_product[$counter]['title'] = 'Full Service Nominee Officer/Manager';
			$order_product[$counter]['price'] = 595;
			$total+=595;
			break;
			
		case 'Attorney' :
			$order_product[$counter]['nid'] = 1097;
			$order_product[$counter]['model'] = 'NATTY';
			$order_product[$counter]['title'] = 'Attorney Nominee Officer/Manager';
			$order_product[$counter]['price'] = 1495;
			$total+=1495;
			break;
	}
}
/*
 * Create Articles
 */
function wysos_order_import_articles($orderid) {
	if (! is_numeric($orderid)) {
	// We will just show a standard "access denied" page in this case.
	return drupal_access_denied();
	} 
	// Get Order by OrderID
	$fed_order = db_query("SELECT fed_www.orders.* FROM fed_www.orders WHERE fed_www.orders.OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
	//$fed_order = db_fetch_array($result);
	
	// Set values based on order type
	include_once('wysos.util.inc');
	switch($fed_order->OrderType) {
		case 'PROFITCORP' :
			$doc_name = 'Articles-'.$fed_order->OrderType.'.odt';
			$fed_profitcorporder = db_query("SELECT * FROM fed_www.profitcorporders WHERE fed_www.profitcorporders.OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
			//$fed_profitcorporder = db_fetch_array($result2);
			$company_name = $fed_profitcorporder->CompanyName;
			$GLOBALS['company_name'] = $fed_profitcorporder->CompanyName;
			$GLOBALS['num_shares'] = $fed_profitcorporder->NumShares;
			$GLOBALS['share_class'] = $fed_profitcorporder->ShareClass;
			if($fed_profitcorporder->SharePar == 'No Par Value')
				$GLOBALS['share_par'] = 'No';
			else
				$GLOBALS['share_par'] = $fed_profitcorporder->SharePar;
			if($fed_profitcorporder->NomineeType == 'None') {
				$data = array();
				if($fed_profitcorporder->PresidentCountry == 'US') {
					$data[] = array('title'=>'President','name'=>$fed_profitcorporder->PresidentName, 'address'=>$fed_profitcorporder->PresidentAddress.
						', '.$fed_profitcorporder->PresidentCity.', '.$fed_profitcorporder->PresidentState.' '.$fed_profitcorporder->PresidentZip);
				} else {
					$data[] = array('title'=>'President','name'=>$fed_profitcorporder->PresidentName, 'address'=>$fed_profitcorporder->PresidentAddress.
						', '.$fed_profitcorporder->PresidentCity.', '.$fed_profitcorporder->PresidentState.' '.$fed_profitcorporder->PresidentZip.' '.
						 wysos_get_country($fed_profitcorporder->PresidentCountry) );
				}
				if(!empty($fed_profitcorporder->VPName)) {
					if($fed_profitcorporder->VPCountry == 'US') {
						$data[] = array('title'=>'Vice-President','name'=>$fed_profitcorporder->VPName, 'address'=>$fed_profitcorporder->VPAddress.
							', '.$fed_profitcorporder->VPCity.', '.$fed_profitcorporder->VPState.' '.$fed_profitcorporder->VPZip);
					} else {
						$data[] = array('title'=>'Vice-President','name'=>$fed_profitcorporder->VPName, 'address'=>$fed_profitcorporder->VPAddress.
							', '.$fed_profitcorporder->VPCity.', '.$fed_profitcorporder->VPState.' '.$fed_profitcorporder->VPZip.' '.
							wysos_get_country($fed_profitcorporder->VPCountry));
					}
				}
				if(!empty($fed_profitcorporder->SecretaryName)) {
					if($fed_profitcorporder->SecretaryCountry == 'US') {
						$data[] = array('title'=>'Secretary','name'=>$fed_profitcorporder->SecretaryName, 'address'=>$fed_profitcorporder->SecretaryAddress.
							', '.$fed_profitcorporder->SecretaryCity.', '.$fed_profitcorporder->SecretaryState.' '.$fed_profitcorporder->SecretaryZip);
					} else {
						$data[] = array('title'=>'Secretary','name'=>$fed_profitcorporder->SecretaryName, 'address'=>$fed_profitcorporder->SecretaryAddress.
							', '.$fed_profitcorporder->SecretaryCity.', '.$fed_profitcorporder->SecretaryState.' '.$fed_profitcorporder->SecretaryZip.' '.
							wysos_get_country($fed_profitcorporder->SecretaryCountry));
					}
				}
			} else {
				// Nominee Service
					$data[] = array('title'=>'President','name'=>'Wyoming Executive Management Corp.', 'address'=>'2510 Warren Ave., Cheyenne, WY 82001');
			}
			break;
			
		case 'LLC' :
			$doc_name = 'Articles-'.$fed_order->OrderType.'.odt';
			$fed_llcorder = db_query("SELECT * FROM fed_www.llcorders WHERE fed_www.llcorders.OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
			//$fed_llcorder = db_fetch_array($result2);
			$company_name = $fed_llcorder->CompanyName;
			if(stripos($company_name,' LLC')===false && stripos($company_name,' LC')===false && stripos($company_name,' Limited Liability Company')===false 
				&& stripos($company_name,' Ltd. Liability Company')===false && stripos($company_name,' L.L.C.')===false && stripos($company_name,' Limited Company')===false) {
				$company_name = trim($company_name).' LLC';
			}
				
			$GLOBALS['company_name'] = $company_name;
			if($fed_llcorder->NomineeType == 'None') {
				$company_name = $fed_llcorder->CompanyName;
				$data = array();
				if($fed_llcorder->Manager1Country == 'US') {
					$data[] = array('name'=>$fed_llcorder->Manager1Name, 'address'=>$fed_llcorder->Manager1Address.
						', '.$fed_llcorder->Manager1City.', '.$fed_llcorder->Manager1State.' '.$fed_llcorder->Manager1Zip);
				} else {
					$data[] = array('name'=>$fed_llcorder->Manager1Name, 'address'=>$fed_llcorder->Manager1Address.
						', '.$fed_llcorder->Manager1City.', '.$fed_llcorder->Manager1State.' '.$fed_llcorder->Manager1Zip.' '.
						wysos_get_country($fed_llcorder->Manager1Country));
				}
				if(!empty($fed_llcorder->Manager2Name)) {
					if($fed_profitcorporder->Manager2Country == 'US') {
						$data[] = array('name'=>$fed_llcorder->Manager2Name, 'address'=>$fed_llcorder->Manager2Address.
							', '.$fed_llcorder->Manager2City.', '.$fed_llcorder->Manager2State.' '.$fed_llcorder->Manager2Zip);
					} else {
						$data[] = array('name'=>$fed_llcorder->Manager2Name, 'address'=>$fed_llcorder->Manager2Address.
							', '.$fed_llcorder->Manager2City.', '.$fed_llcorder->Manager2State.' '.$fed_llcorder->Manager2Zip.' '.
							wysos_get_country($fed_llcorder->Manager2Country));
					}
				}
				if(!empty($fed_llcorder->Manager3Name)) {
					if($fed_llcorder->Manager3Country == 'US') {
						$data[] = array('name'=>$fed_llcorder->Manager3Name, 'address'=>$fed_llcorder->Manager3Address.
							', '.$fed_llcorder->Manager3City.', '.$fed_llcorder->Manager3State.' '.$fed_llcorder->Manager3Zip);
					} else {
						$data[] = array('name'=>$fed_llcorder->Manager3Name, 'address'=>$fed_llcorder->Manager3Address.
							', '.$fed_llcorder->Manager3City.', '.$fed_llcorder->Manager3State.' '.$fed_llcorder->Manager3Zip.' '.
							wysos_get_country($fed_llcorder->Manager3Country));
					}
				}
			} else {
				// Nominee Service
					$data[] = array('name'=>'Wyoming Executive Management Corp.', 'address'=>'2510 Warren Ave., Cheyenne, WY 82001');
			}
			break;
	}
	

	// Open the OpenOffice Template
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');

	$TBS = new clsTinyButStrong;
	$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

	$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name); // Load the archive 'document.odt'.
	$TBS->MergeBlock('a', $data);
	$TBS->Show(OPENTBS_DOWNLOAD, 'Articles-'.$company_name.'.odt');
}
/*
 * Update To Processed
 */
function wysos_order_import_done($orderid) {
	if (! is_numeric($orderid)) {
	// We will just show a standard "access denied" page in this case.
	return drupal_access_denied();
	} 
	
	db_update('fed_www.orders')
		->fields(array(
			'ProcessedDt' => REQUEST_TIME,
		))->condition('OrderId',$orderid)->execute();
	
	drupal_goto('wysos/order_import');
}
/*
 * Update To Processed
 */
function wysos_order_import_resolutions($orderid) {
	if (! is_numeric($orderid)) {
	// We will just show a standard "access denied" page in this case.
	return drupal_access_denied();
	} 
	
	// Get Order by OrderID
	$fed_order = db_query("SELECT fed_www.orders.* FROM fed_www.orders WHERE fed_www.orders.OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
	//$fed_order = db_fetch_array($result);
	
	// Set values based on order type
	include_once('wysos.util.inc');
	switch($fed_order->OrderType) {
		case 'PROFITCORP' :
			$fed_profitcorporder = db_query("SELECT * FROM fed_www.profitcorporders WHERE fed_www.profitcorporders.OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
			//$fed_profitcorporder = db_fetch_array($result2);
			$doc_name = 'Resign-'.$fed_order->OrderType.'-'.$fed_profitcorporder->NomineeType.'.odt';
			$uid = db_query("SELECT uid FROM uc_orders WHERE order_id=:oid",array(':oid'=> $fed_order->uc_orderid))->fetchField();
			$account = user_load($uid);
			
			$company_name = $fed_profitcorporder->CompanyName;
			$GLOBALS['company_name'] = $fed_profitcorporder->CompanyName;
			$GLOBALS['username'] = $account->name;
			$GLOBALS['password'] = $account->name.'11';
			$GLOBALS['new_director'] = $fed_order->CustomerFirstName.' '.$fed_order->CustomerLastName;
			$output = 'company_name='.$GLOBALS['company_name'].'<br>username='.$GLOBALS['username'].'<br>password='.$GLOBALS['password'].'<br>new_director='.$GLOBALS['new_director'];
			drupal_set_message($output);
			break;
			
		case 'LLC' :
			$fed_llcorder = db_query("SELECT * FROM fed_www.llcorders WHERE fed_www.llcorders.OrderId=:orderid",array(':orderid'=>$orderid))->fetchObject();
			//$fed_llcorder = db_fetch_array($result2);
			$doc_name = 'Resign-'.$fed_order->OrderType.'-'.$fed_llcorder->NomineeType.'.odt';
			$uid = db_query('SELECT uid FROM uc_orders WHERE order_id=:ofd', array(':ofd'=>$fed_order->uc_orderid))->fetchField();
			$account = user_load($uid);
			
			$company_name = $fed_llcorder->CompanyName;
			if(stripos($company_name,' LLC')===false && stripos($company_name,' LC')===false && stripos($company_name,' Limited Liability Company')===false 
				&& stripos($company_name,' Ltd. Liability Company')===false && stripos($company_name,' L.L.C.')===false) {
				$company_name = trim($company_name).' LLC';
			}
			$GLOBALS['company_name'] = $company_name;
			$GLOBALS['username'] = $account->name;
			$GLOBALS['password'] = $account->name.'11';
			$GLOBALS['new_director'] = $fed_order->CustomerFirstName.' '.$fed_order->CustomerLastName;
			break;
	}

	// Open the OpenOffice Template
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');

	$TBS = new clsTinyButStrong;
	$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

	$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name); // Load the archive 'document.odt'.
	$TBS->Show(OPENTBS_DOWNLOAD, 'Resolutions-'.$company_name.'.odt');
}
/*
 * Delete Imported Order
 */
function wysos_order_import_delete($weborderid) {
	if (! is_numeric($weborderid)) {
	// We will just show a standard "access denied" page in this case.
	return drupal_access_denied();
	} 
	$orderid = db_query("SELECT uc_orderid FROM fed_www.orders WHERE OrderId=:weborderid", array(':weborderid'=>$weborderid))->fetchField();
	
	if($orderid) {
		$result = db_query("DELETE FROM uc_orders WHERE order_id=%d", $orderid);
		
		$result = db_delete('uc_orders')
			->condition('order_id',$orderid,'=')->execute();
		$result = db_delete('uc_order_admin_comments')
			->condition('order_id',$orderid,'=')->execute();		
		$result = db_delete('uc_order_comments')
			->condition('order_id',$orderid,'=')->execute();
		$result = db_delete('uc_order_line_items')
			->condition('order_id',$orderid,'=')->execute();
		$result = db_delete('uc_order_log')
			->condition('order_id',$orderid,'=')->execute();
		$result = db_delete('uc_order_products')
			->condition('order_id',$orderid,'=')->execute();
				
		$result = db_update('fed_www.orders')
				->fields(array(
					'Status' => NULL,
					'uc_orderid' => NULL,
				))->condition('OrderId',$weborderid)->execute();
	}
	drupal_goto('wysos/order_import');
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


