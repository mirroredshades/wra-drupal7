<?php 

// Apply for Office
function wysos_office_apply_form($form, &$form_state, $office_no) {
	include_once('wysos.util.inc');

	drupal_set_title('Apply to lease an Executive Suite');
	$rowOffice = db_query("SELECT * from wra_offices WHERE office_no=:office_no",array(':office_no'=>$office_no))->fetchObject();
	
	$form_state['storage']['rowOffice'] = $rowOffice;
	
	
	
	$form['office_info'] = array(
		'#title' => t("Office Information"),
		'#prefix' => '<div id="office_info">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
	);
	$form['office_info']['office_no'] = array(
		'#title' => t('Office No'),
		'#type' => 'textfield',
		'#size' => '4',
		'#maxlength' => '4',
		'#default_value' => $office_no,
		'#disabled' => true, 
	);
	$form['office_info']['start_month'] = array(
		'#title' => t('Move in Month'),
		'#type' => 'select',
		'#options' => array(
			'January' => 'January',
			'February' => 'February',
			'March' => 'March',
			'April' => 'April',
			'May' => 'May',
			'June' => 'June',
			'July' => 'July',
			'August' => 'August',
			'September' => 'September',
			'October' => 'October',
			'November' => 'November',
			'December' => 'December' ),
		'#default_value' => date('F'),
	);
	$form['office_info']['lease_term'] = array(
		'#title' => t('Lease Term'),
		'#type' => 'radios',
		'#options' => array(
			'month' => 'Month to Month (/month)',
			'year' => '12 Month (/month)',
			'2year' => '24 Month (/month)'),
		'#default_value' => isset($form_state['values']['lease_term'])?$form_state['values']['lease_term']:'',
	);
	$form['customer_info'] = array(
		'#title' => t("Customer Info"),
		'#prefix' => '<div id="customer_info">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
	);
	$form['customer_info']['email'] = array(
		'#title' => t("Email"),
		'#type' => 'textfield',
		'#size' => '60',
		'#default_value' => isset($form_state['values']['email'])?$form_state['values']['email']:'',
	);
	$form['customer_info']['first_name'] = array(
		'#title' => t('First Name'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['first_name'])?$form_state['values']['first_name']:'',
	);
	$form['customer_info']['last_name'] = array(
		'#title' => t('Last Name'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['last_name'])?$form_state['values']['last_name']:'',
	);
	$form['customer_info']['company_name'] = array(
		'#title' => t('Company'),
		'#type' => 'textfield',
		'#size' => '40',
		'#maxlength' => '254',
		'#default_value' => isset($form_state['values']['company_name'])?$form_state['values']['company_name']:'',
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
	
	$form['payment_info'] = array(
		'#title' => t("Payment"),
		'#prefix' => '<div id="payment_info">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
	);
	$form['payment_info']['payment_type'] = array(
		'#title' => t('Payment Type'),
		'#type' => 'radios',
		'#options' => array(
			'check' => 'Check',
			'credit' => 'Credit Card' ,
			'wire' => 'Bank Wire') ,
		'#default_value' => isset($form_state['values']['payment_type'])?$form_state['values']['payment_type']:'',
	);
		
	$form['actions-submit'] = array(
		'#type' => 'submit',
		'#value' => 'Submit'
	);
	return $form;		
}


function wysos_office_apply_form_validate($form, &$form_state) {
}

function wysos_office_apply_form_submit($form, &$form_state) {

	// Is email a user in the system already?
	$uid = db_query("SELECT uid FROM users WHERE mail= :email",array(':email'=>$form_state['values']['email']))->fetchField();
	drupal_set_message('uid: '.$uid);
	
	if($uid  > 0)
	{
	} else {
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
		
		user_multiple_role_edit(array($nuser->uid), 'add_role', 4);	
	}
	$client = user_load($uid);	
	$address = db_query("SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
		FROM users
		INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
		INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
		WHERE users.uid = :uid",array(':uid'=>$uid))->fetchObject();
	
	
	// Create an Invoice
	include_once('wysos.util.inc');
	$order_id = wysos_util_create_invoice_uid($uid);
	$total = 0;
	$num_products = 0;
	$rent = 0;
	$payment_method = '';
	// Create Product
	$data = array("attributes" => array("Office Number"=>array($form_state['storage']['rowOffice']->office_no),
				"Month" => $form_state['values']['start_month']));
	switch($form_state['values']['lease_term']) {
		case 'month' :
			$rent = $form_state['storage']['rowOffice']->m2m_lease;
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 1803,
							'model' => 'OFFICE',
							'title' => 'Office Rent',
							'qty' => 1,
							'price' => $rent,
							'data' => serialize($data),
						))->execute();	
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 0,
							'model' => '',
							'title' => 'Month to Month Lease',
							'qty' => 1,
							'price' => 0,
						))->execute();	
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 1834,
							'model' => 'Deposit',
							'title' => 'Deposit',
							'qty' => 1,
							'price' => $rent,
						))->execute();
			
			break;
			
		case 'year' :
			$rent = $form_state['storage']['rowOffice']->year_lease;
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 1803,
							'model' => 'OFFICE',
							'title' => 'Office Rent',
							'qty' => 1,
							'price' => $rent,
							'data' => serialize($data),
						))->execute();	
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 0,
							'model' => '',
							'title' => '12 Month Lease',
							'qty' => 1,
							'price' => 0,
						))->execute();	
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 1834,
							'model' => 'Deposit',
							'title' => 'Deposit',
							'qty' => 1,
							'price' => $rent,
						))->execute();		
			break;
			
		case '2year' :
			$rent = $form_state['storage']['rowOffice']->year_lease;
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 1803,
							'model' => 'OFFICE',
							'title' => 'Office Rent',
							'qty' => 1,
							'price' => $rent,
							'data' => serialize($data),
						))->execute();		
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 0,
							'model' => '',
							'title' => '24 Month Lease',
							'qty' => 1,
							'price' => 0,
						))->execute();
			$result = db_insert('uc_order_products')
						->fields(array(
							'order_id' => $order_id,
							'nid' => 1834,
							'model' => 'Deposit',
							'title' => 'Deposit',
							'qty' => 1,
							'price' => $rent,
						))->execute();
			break;
	}
	
	switch($form_state['values']['payment_type']) {
		case 'check' :
			$payment_method = 'check';
			break;
			
		case 'credit' :
			$payment_method = 'credit';
			break;
			
		case 'wire' :
			$payment_method = 'wire';
			break;
	}
	
	// Update the uc_order with number products, total
	$result = db_update('uc_orders')
			->fields(array(
				'product_count' => 3,
				'order_total' => $rent*2,
				'payment_method' => $payment_method,
			))->condition('order_id',$order_id,'=')
			->execute();
	
	$order = uc_order_load($order_id);
	$params = array('order' => $order);

	$params = array(array());
	$params['@order_id'] = $order_id;
	$params['@uid'] = $uid;
	$params['@email'] = $form_state['values']['email'];
	$params['@office_no'] = $form_state['storage']['rowOffice']->office_no;
	foreach ($address as $key => $val){ 
		$params['@'.$key] = $val;	
	}
	// Email info@17thandcentral.com
	$message = array(
	  'to' => 'info@17thandcentral.com',
	  'subject' => 'Rental Application for Office @office_no',
	  'body' => "Hello,\n\n@first_name @last_name just applied for a lease for office @office_no\n\nhttps://17thandcentral.com/admin/store/orders/@order_id\nhttps://17thandcentral.com/wysos/user/@uid\n@email\n",
	  'headers' => array('From' => 'info@wyomingregisteredagent.com'),
	);
	//drupal_mail_send($message); 
	drupal_goto('/');

}

// Display Available Offices
function wysos_office_available()
{
   	drupal_set_title('Available Offices and Workstations');
	//$output = array();
	$output = '<h2>Lower Level<br><img src=\'/images/FloorplanL1.jpg\' alt=\'Lower Level Floorplan\' /></h2>';

	// Table Header
	$headers = array(
		array(
			'data' => t('Office'),
			'field' => 'office_no'),
		array(
			'data' => t('Floorplan'),
			'field' => 'floorplan_office'),
		array(
			'data' => t('Type'),
			'field' => 'type'),
		array(
			'data' => t('M 2 M Lease'),
			'field' => 'm2m_lease'),
		array(
			'data' => t("12 Mo Lease"),
			'field' => 'year_lease'),
		array(
			'data' => t("Available Date"),
			'field' => 'available'),	
		);
		
		$result = db_query("SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.type, wra_offices.m2m_lease, wra_offices.year_lease, wra_suites.contract_end AS available
FROM wra_offices
LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
INNER  JOIN wra_suites ON wra_suites.suite_id = wra_office_suite.suite_id
WHERE wra_offices.office_no IN (SELECT office_no FROM wra_office_suite INNER JOIN wra_suites ON wra_office_suite.suite_id = wra_suites.suite_id WHERE wra_suites.contract_end BETWEEN UNIX_TIMESTAMP() AND UNIX_TIMESTAMP()+5184000) AND wra_offices.level = 'Lower'
UNION
SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.type, wra_offices.m2m_lease, wra_offices.year_lease, 'Now' AS available
            FROM wra_offices
            LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
			WHERE wra_offices.office_no NOT IN (SELECT office_no FROM wra_office_suite) AND wra_offices.level = 'Lower'
 ORDER BY office_no")->fetchAll();
 
		foreach($result as $officeRow)
		{
			$row[]  = array(
				'style' => 'background-color: LightGrey;',
				'data'  => 	array(array(
						'data' => $officeRow->office_no,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->floorplan_office,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->type,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->m2m_lease,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->year_lease,
						'style' => 'text-align:right;'),
					array('data' => is_numeric($officeRow->available) ? date('m-d-Y',$officeRow->available) : $officeRow->available,
						'style' => 'text-align:right;'),
					));
		}
	//$output .= theme('table', $headers, $row, $attributes);
	
	$output .= theme('table',array('header'=>$headers,'rows'=>$row));
	
	// Main Level
	$output .= '<br /><h2>Main Level<br /><img src="/public/images/Floorplan_Main1.jpg" alt="Main Level Floorplan" /></h2>';
	$row = array();

		$result = db_query("SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.type, wra_offices.m2m_lease, wra_offices.year_lease, wra_suites.contract_end AS available
FROM wra_offices
LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
INNER  JOIN wra_suites ON wra_suites.suite_id = wra_office_suite.suite_id
WHERE wra_offices.office_no IN (SELECT office_no FROM wra_office_suite INNER JOIN wra_suites ON wra_office_suite.suite_id = wra_suites.suite_id WHERE wra_suites.contract_end BETWEEN UNIX_TIMESTAMP() AND UNIX_TIMESTAMP()+5184000) AND wra_offices.level = 'Main'
UNION
SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.type, wra_offices.m2m_lease, wra_offices.year_lease, 'Now' AS available
            FROM wra_offices
            LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
			WHERE wra_offices.office_no NOT IN (SELECT office_no FROM wra_office_suite) AND wra_offices.level = 'Main'
 ORDER BY office_no")->fetchAll();
		foreach($result as $officeRow)
		{
			$row[]  = array(
				'style' => 'background-color: LightGrey;',
				'data'  => 	array(array(
						'data' => $officeRow->office_no,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->floorplan_office,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->type,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->m2m_lease,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->year_lease,
						'style' => 'text-align:right;'),
					array('data' => is_numeric($officeRow->available) ? date('m-d-Y',$officeRow->available) : $officeRow->available,
						'style' => 'text-align:right;'),
					));
		}
	//$output .= theme('table', $headers, $row, $attributes);
	$output .= theme('table',array('header'=>$headers,'rows'=>$row));
	// Mezzanine Level
	$output .= '<br /><h2>Mezzanine Level<br /><img src="/public/images/Floorplan_mezz.jpg" alt="Lower Level Floorplan" /></h2>';
	$row = array();

	$result = db_query("SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.type, wra_offices.m2m_lease, wra_offices.year_lease, wra_suites.contract_end AS available
FROM wra_offices
LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
INNER  JOIN wra_suites ON wra_suites.suite_id = wra_office_suite.suite_id
WHERE wra_offices.office_no IN (SELECT office_no FROM wra_office_suite INNER JOIN wra_suites ON wra_office_suite.suite_id = wra_suites.suite_id WHERE wra_suites.contract_end BETWEEN UNIX_TIMESTAMP() AND UNIX_TIMESTAMP()+5184000) AND wra_offices.level = 'Mezzanine'
UNION
SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.type, wra_offices.m2m_lease, wra_offices.year_lease, 'Now' AS available
            FROM wra_offices
            LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
			WHERE wra_offices.office_no NOT IN (SELECT office_no FROM wra_office_suite) AND wra_offices.level = 'Mezzanine'
 ORDER BY office_no")->fetchAll();
		foreach($result as $officeRow)
		{
			$row[]  = array(
				'style' => 'background-color: LightGrey;',
				'data'  => 	array(array(
						'data' => $officeRow->office_no,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->floorplan_office,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->type,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->m2m_lease,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->year_lease,
						'style' => 'text-align:right;'),
					array('data' => is_numeric($officeRow->available) ? date('m-d-Y',$officeRow->available) : $officeRow->available,
						'style' => 'text-align:right;'),
					));
		}
	//$output .= theme('table', $headers, $row, $attributes);
	$output .= theme('table',array('header'=>$headers,'rows'=>$row));
	return($output);
}

function wysos_office_list()
{
   	drupal_set_title('Office List');
   
	// Table Header
	$headers = array(
		array(
			'data' => t('Office'),
			'field' => 'office_no'),
		array(
			'data' => t('Company'),
			'field' => 'company_name',
			'sort' => 'asc'),
		array(
			'data' => t('User'),
			'field' => 'username'),
		array(
			'data' => t('Floorplan'),
			'field' => 'floorplan_office'),
		array(
			'data' => t('Level'),
			'field' => 'level'),
		array(
			'data' => t('Type'),
			'field' => 'type'),
		array(
			'data' => t('Phone No'),
			'field' => 'phone_no'),
		array(
			'data' => t('Extension'),
			'field' => 'phone_extension'),
		array(
			'data' => t('Last Invoice'),
			'field' => 'last_invoice'),
		array(
			'data' => t('Balance Due'),
			'field' => 'balance_due'),
			'options'
		);
		
		$result = db_query("SELECT wra_offices.office_no, wra_companies.company_name,wra_offices.floorplan_office, wra_offices.level,wra_offices.type, wra_suites.suite_id, '' AS phone_no, '' AS phone_extension, users.name AS username, users.uid, (SELECT o.order_id FROM uc_orders o INNER JOIN uc_order_products p ON o.order_id = p.order_id WHERE o.uid=users.uid AND p.model='OFFICE' AND p.data LIKE CONCAT('%%', wra_offices.office_no,'%%') ORDER BY o.created DESC LIMIT 1) AS last_order, (SELECT SUM(order_total) FROM uc_orders WHERE uid=users.uid) -
IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id IN(SELECT order_id FROM uc_orders WHERE uid=users.uid)),0) AS balance_due 
FROM wra_offices
LEFT JOIN wra_office_suite ON wra_offices.office_no = wra_office_suite.office_no
INNER JOIN wra_suites ON wra_suites.suite_id = wra_office_suite.suite_id
INNER JOIN wra_companies ON wra_companies.suite_id = wra_office_suite.suite_id 
INNER JOIN users ON users.uid = wra_suites.uid
ORDER BY wra_companies.company_name")->fetchAll();

		$rowno = 0;
		foreach($result as $officeRow)
		{
			$rowno++;
			$row[]  = array(
//				'style' => (($rowno % 2) ? 'background-color: LightGrey;' : 'background-color: White;'),
				'data'  => 	array(array(
						'data' => $officeRow->office_no,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->company_name,
						'style' => 'font-weight: bold;text-align:left;'),
					array('data' => l($officeRow->username,'wysos/user/'.$officeRow->uid),
						'style' => 'font-weight: bold;text-align:left;'),
					array('data' => $officeRow->floorplan_office,
						'style' => 'text-align:left;'),
					array('data' => $officeRow->level,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->type,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->phone_no,
						'style' => 'text-align:right;'),
					array('data' => $officeRow->phone_extension,
						'style' => 'text-align:right;'),
					array('data' => l($officeRow->last_order, 'admin/store/orders/'.$officeRow->last_order),
						'style' => 'text-align:right;'),
					array('data' => uc_currency_format($officeRow->balance_due),
						'style' => 'text-align:right;'),
					array('data' => '<font size=-3>'.l('Create_Rent_Bill','wysos/office/create_invoice/'.$officeRow->suite_id).'</font>',
						'style' => 'text-align:right;'),
					));
		}
	//$output .= theme('table', $headers, $row, $attributes);
	$output = theme('table',array('header'=>$headers,'rows'=>$row)); 
	$output .= "<br /><h2>Lower Level<br /><img src='/public/images/FloorplanL1.jpg' alt='Lower Level Floorplan' /></h2>";
	$output .= "<br /><h2>Main Level<br /><img src='/public/images/Floorplan_Main1.jpg' alt='Main Level Floorplan' /></h2>";
	$output .= "<br /><h2>Mezzanine Level<br /><img src='/public/images/Floorplan_mezz.jpg' alt='Lower Level Floorplan' /></h2>";

	return($output);
}

function wysos_office_create_invoice_form($form, &$form_state, $suite_id)
{
	$rowSuite = db_query("SELECT street_address, suite_no, uid FROM wra_suites WHERE suite_id =:suite_id",array(':suite_id'=>$suite_id))->fetchObject();
	//$rowSuite = db_fetch_array($result1);
	
	$form_state['storage']['suite_id'] = $suite_id;
	$form_state['storage']['uid'] = $rowSuite->uid;
	$form_state['storage']['referer'] = $_SERVER["HTTP_REFERER"];
	$copy_code = db_query("SELECT value FROM wra_suite_resources WHERE suite_id=:suite_id and resource_type='copy_code'",array(':suite_id'=>$suite_id))->fetchField();
	$toll_free = db_query("SELECT value FROM wra_suite_resources WHERE suite_id=:suite_id and resource_type='toll_free'", array(':suite_id'=>$suite_id))->fetchField();
	$company_name = db_query("SELECT company_name FROM wra_companies WHERE suite_id = :suite_id LIMIT 1", array('suite_id'=>$suite_id))->fetchField();
	drupal_set_title($company_name);
	$form_state['storage']['company_name'] = $company_name;
	
	$result2 = db_query("SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.level, wra_offices.current_rent, wra_offices.type FROM wra_offices
INNER JOIN wra_office_suite ON wra_office_suite.office_no = wra_offices.office_no
WHERE wra_office_suite.suite_id=:suite_id", array(':suite_id'=>$suite_id))->fetchAll();

	$headers = array(
		array(
			'data' => t('Office'),
			'field' => 'office_no'),
		array(
			'data' => t('Level'),
			'field' => 'level'),
		array(
			'data' => t('Floorplan'),
			'field' => 'floorplan_office'),
		array(
			'data' => t('Type'),
			'field' => 'type'),
		array(
			'data' => t('Rent'),
			'field' => 'current_rent')
		);
	$row = array();
	foreach($result2 as $rowOffice)
	{
		$form_state['storage']['offices'][] = $rowOffice;
		$row[]  = array(
			'data'  => 	array(array(
					'data' => $rowOffice->office_no, 
					'style' => 'font-weight: bold;text-align:left;'),
				array('data' => $rowOffice->level,
					'style' => 'text-align:left;'),
				array('data' => $rowOffice->floorplan_office,
					'style' => 'text-align:left;'),
				array('data' => $rowOffice->type,
					'style' => 'text-align:left;'),
				array('data' => $rowOffice->current_rent,
					'style' => 'text-align:left;'),
		));
	}
	$form['item1'] = array(
		'#type' => 'item',
		'#markup' => $rowSuite->street_address.' #'.$rowSuite->suite_no
	);
	$form['item2'] = array(
		'#type' => 'item',
		'#title' => 'Copy Code',
		'#markup' => $copy_code
	);
	if($toll_free) {
		$form['toll_free'] = array(
			'#type' => 'item',
			'#title' => 'Toll Free Number',
			'#markup' => $toll_free
		);
	}
	$form['value'] = array(
		'#type' => 'item',
		'#markup' => theme('table',array('header'=>$headers,'rows'=>$row))
	);
  	$form['month'] = array(
		'#type' => 'select',
		'#title' => "Month",
		'#options' => array(1 => t('01 - January'), 2 => t('02 - February'),
							3 => t('03 - March'), 4 => t('04 - April'),
							5 => t('05 - May'), 6 => t('06 - June'),
							7 => t('07 - July'), 8 => t('08 - August'),
							9 => t('09 - September'), 10 => t('10 - October'),
							11 => t('11 - November'), 12 => t('12 - December')
					  ),
		'#default_value' => date('n'),
	);
	$form['bw_copies'] = array(
		'#title' => t("Black & White Copies"),
		'#type' => 'textfield',
		'#size' => '5',
	);
	$form['color_copies'] = array(
		'#title' => t("Color Copies"),
		'#type' => 'textfield',
		'#size' => '5',
	);
	$form['scans'] = array(
		'#title' => t("Scans"),
		'#type' => 'textfield',
		'#size' => '5',
	);
	$form['outgoing_fax'] = array(
		'#title' => t("Outgoing Faxes"),
		'#type' => 'textfield',
		'#size' => '5',
	);
	$form['postage'] = array(
		'#title' => t("Postage"),
		'#type' => 'textfield',
		'#size' => '7',
	);
	if($toll_free) {
		$form['tollfree_minutes'] = array(
			'#title' => t("Tollfree Minutes"),
			'#type' => 'textfield',
			'#size' => '5',
		);
	}
	$form['administrative_hours'] = array(
		'#title' => t("Administrative Hours"),
		'#type' => 'textfield',
		'#size' => '5',
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Create Invoice'
	);
	return $form;
}

function  wysos_office_create_invoice_form_validate($form, &$form_state)
{
}


function  wysos_office_create_invoice_form_submit($form, &$form_state)
{
	$suite_id = $form_state['storage']['suite_id'];
	$uid = $form_state['storage']['uid'];
	$rowSuite = db_query("SELECT * FROM wra_suites WHERE suite_id =:suite_id",array(':suite_id'=>$suite_id))->fetchObject();
	//$rowSuite = db_fetch_array($result1);
	
	$suite_user = user_load($uid);
	
	// Get uc_address
	$address = db_query("SELECT uc_addresses.* FROM uc_addresses
	INNER JOIN uc_addresses_defaults ON uc_addresses.aid = uc_addresses_defaults.aid
	WHERE uc_addresses_defaults.uid=:uid",array(':uid'=>$suite_user->uid))->fetchObject();
	
	//$address = db_fetch_object($add_result);
	
	$result = db_insert('uc_orders')
			->fields(array(
			'uid' => $uid,
			'order_status' => 'pending',
			'order_total' => 0,
			'product_count' => 1,
			'primary_email' => $suite_user->mail,
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
	
	if(!$result) {
		return;
	}
	$orderid = $result;
	$total = 0;
	
	// Add Office Rent Item
	$result2 = db_query("SELECT wra_offices.office_no, wra_offices.floorplan_office, wra_offices.level, wra_offices.current_rent, wra_offices.type FROM wra_offices
INNER JOIN wra_office_suite ON wra_office_suite.office_no = wra_offices.office_no
WHERE wra_office_suite.suite_id=:suite_id", array(':suite_id'=>$suite_id))->fetchAll();

	foreach($result2 as $rowOffice)
	{
		$months = array('','January','February','March','April','May','June','July','August','September','October','November','December');
		$data = array('attributes' => array('Office Number' => $rowOffice->office_no, 'Month' => $months[$form_state['values']['month']]));

		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1803,
					'model' => 'OFFICE',
					'title' => 'Office Rent',
					'qty' => 1,
					'price' => $rowOffice->current_rent,
					'data' => serialize($data)	
				))->execute();
			
		$total += $rowOffice->current_rent;
	}
	
	if($form_state['values']['bw_copies'] > 0) {
		$copies = $form_state['values']['bw_copies'];
		if($copies < 500) 
			$cost_copy = .10;
		else if($copies > 500 AND $copies < 1501)
			$cost_copy = .08;
		else
			$cost_copy = .06;
			
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1805,
					'model' => 'BWCOPY',
					'title' => 'Black & White Copies',
					'qty' => $copies,
					'price' => $cost_copy,
				))->execute();
		$total += ($copies * $cost_copy);
	}
	if($form_state['values']['color_copies'] > 0) {
		$copies = $form_state['values']['color_copies'];
		$cost_copy = .25;
		
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1806,
					'model' => 'COLORCOPY',
					'title' => 'Color Copies',
					'qty' => $copies,
					'price' => $cost_copy,
				))->execute();
		$total += ($copies * $cost_copy);
	}
	if($form_state['values']['scans'] > 0) {
		$copies = $form_state['values']['scans'];
		if($copies <= 10) 
			$cost_copy = .25;
		else
			$cost_copy = .10;
			
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1816,
					'model' => 'SCANS',
					'title' => 'Document Scanning',
					'qty' => $copies,
					'price' => $cost_copy,
				))->execute();
		$total += ($copies * $cost_copy);
	}
	if($form_state['values']['outgoing_fax'] > 0) {
		$copies = $form_state['values']['outgoing_fax'];
		$cost_copy = .10;
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1810,
					'model' => 'FAXOUT',
					'title' => 'Outgoing Faxes',
					'qty' => $copies,
					'price' => $cost_copy,
				))->execute();
		$total += ($copies * $cost_copy);
	}
	if($form_state['values']['postage'] > 0) {
		$cost = $form_state['values']['postage'] * 1.2;
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1233,
					'model' => 'SHIP',
					'title' => 'Postage and Handling',
					'qty' => 1,
					'price' => $cost,
				))->execute();
		$total += $cost;
	}
	if($form_state['values']['tollfree_minutes'] > 0) {
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1807,
					'model' => '800NUMBER',
					'title' => 'Toll Free Number Monthly',
					'qty' => 1,
					'price' => 10.00,
				))->execute();		
		$copies = $form_state['values']['tollfree_minutes'];
		$cost_copy = .10;
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1808,
					'model' => '800MINUTES',
					'title' => 'Toll Free Number Monthly',
					'qty' => $copies,
					'price' => $cost_copy,
				))->execute();		
		$total += 10.00 + ($copies * $cost_copy);
	}
	if($form_state['values']['administrative_hours'] > 0) {
		$cost = $form_state['values']['administrative_hours'] * 20.00;
		
		$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1815,
					'model' => 'SECRETARY',
					'title' => 'Administrative/Secretarial Hours',
					'qty' => $form_state['values']['administrative_hours'],
					'price' => 20.00,
				))->execute();	
			
		$total += $cost;
	}
	
	//$result = db_query("UPDATE uc_orders SET order_total=%f WHERE order_id=%d", $total, $orderid);
	$result = db_update('uc_orders')
				->fields(array(
					'order_total'=> $total,
				))
				->condition('order_id',$orderid,'=')
				->execute();	
	drupal_set_message('Order '.l($orderid,'admin/store/orders/'.$orderid).' created.');
	drupal_goto('admin/store/orders/'.$orderid);
}

function wysos_office_lease_form($form, &$form_state, $office_no = NULL)
{
	$result = db_query("SELECT office_no, level, floorplan_office, type FROM wra_offices ORDER BY office_no")->fetchAll();
	
	foreach($result as $row) {
		$option[$row->office_no] = $row->office_no . ' '.$row->type.' ('.$row->level.' '.$row->floorplan_office.')';
	}
	$form['office_no'] = array(
		'#title' => t("Office Number"),
		'#type' => 'select',
		'#options' => $option
	);
	if(empty($office_no)) 
		$form['office_no']['#disabled'] = false;
	else {
		$form['office_no']['#disabled'] = true;
		$form['office_no']['#default_value'] = $office_no; };
    $form['company_name'] = array(
		'#title' => t('Company Name'),
        '#type' => 'textfield',
        '#size' => '50',
		'#maxlength' => '254',
        '#autocomplete_path' => 'wysos/autocomplete/all_companies',
		'#required' => true );
    $form['title'] = array(
		'#title' => t('Title'),
        '#type' => 'textfield',
        '#size' => '50',
		'#maxlength' => '254',
        '#default_value' => 'President',
		'#required' => true );
	$form['user_name'] = array(
		'#title' => t('User Name'),
		'#type' => 'textfield',
		'#size' => '30',
		'#maxlength' => '60',
		'#autocomplete_path' => 'user/autocomplete',
		'#suffix' => ' <b>-OR-</b>');
	$form['address_info'] = array(
		'#title' => t("New User"),
		'#prefix' => '<div id="billing_info">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
	);

    $form['address_info']['first_name'] = array(
		'#title' => t('First Name'),
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '254',
        '#default_value' => '');
    $form['address_info']['last_name'] = array(
		'#title' => t('Last Name'),
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '254',
        '#default_value' => '',
	);
    $form['address_info']['street1'] = array(
		'#title' => t('Address'),
        '#type' => 'textfield',
        '#size' => '60',
		'#maxlength' => '254',
        '#default_value' => '',
	);
    $form['address_info']['street2'] = array(
        '#type' => 'textfield',
        '#size' => '60',
		'#maxlength' => '254',
        '#default_value' => ''
	);
    $form['address_info']['city'] = array(
		'#title' => t('City'),
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '254',
        '#default_value' => ''
	);

	if (isset($_POST['country'])) {
    	$country_id = intval($_POST['country']);
  	} else {
    	$country_id = NULL;
  	}
	//$form['address_info']['country'] = uc_country_select(uc_get_field_name('country'));
	//$form['address_info']['zone'] = uc_zone_select(uc_get_field_name('zone'), NULL, NULL, $country_id);
	
	$countries = db_query("SELECT country_id,country_name FROM uc_countries")->fetchAll();
	$country_list = array(); 
	foreach($countries as $country){
		$country_list[$country->country_id] = $country->country_name;
	}
	$form['address_info']['country'] = array(
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
	$form['address_info']['zone'] = array(
		'#type' => 'select',
		'#options' => $zone_list,
		'#title' => t('State/Province'),
		'#id' => 'tzone', 
	);
	$form['address_info']['zone_load'] = array(
		'#type' => 'item',
	);
	
	
	$form['address_info']['postal_code'] = array(
		'#title' => t('Zip/Postal Code'),
        '#type' => 'textfield',
        '#size' => '12',
		'#maxlength' => '40',
        '#default_value' => '',
	);
    $form['address_info']['phone'] = array(
		'#title' => t('Phone'),
        '#type' => 'textfield',
        '#size' => '14',
		'#maxlength' => '30',
        '#default_value' => '',
	);
    $form['address_info']['email'] = array(
		'#title' => t('Email'),
        '#type' => 'textfield',
        '#size' => '50',
		'#maxlength' => '254',
        '#default_value' => '',
	);
    $form['rent_price'] = array(
		'#title' => t('Rent $'),
        '#type' => 'textfield',
        '#size' => '10',
		'#maxlength' => '20',
        '#default_value' => '',
		'#required' => true	);
    $form['deposit'] = array(
		'#title' => t('Deposit $'),
        '#type' => 'textfield',
        '#size' => '10',
		'#maxlength' => '20',
        '#default_value' => '',
		'#required' => true	);
	$form['lease_type'] = array(
		'#title' => t('Lease Type'),
        '#type' => 'select',
        '#options' => array(''=>'',"Month To Month"=>"Month To Month","6 Month"=>"6 Month","12 Month"=>"12 Month","24 Month"=>"24 Month"),
		'#default_value' => '',
        '#required' => TRUE,
	);
	$form['lease_start'] = array(
		'#title' => t('Lease Start Date'),
        '#type' => 'date',
        '#required' => TRUE,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Create Invoice'
	);
	return $form;
}

function  wysos_office_lease_form_validate($form, &$form_state)
{
}

function  wysos_office_lease_form_submit($form, &$form_state)
{
	// Create User
	// Load or Create the user
	if(empty($form_state['values']['user_name'])) {
		// New User
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
		user_multiple_role_edit(array($nuser->uid), 'add_role', 4);	
		$client = user_load($uid);
	} else {
		// Load Existing User
		$clients = user_load_multiple(array(), array('name' => $form_state['values']['user_name']));
		foreach($clients as $client){
		 $uid = $client->uid;
		}
	}
	$address = db_query("SELECT users.name AS user_name, users.mail AS email, uc_addresses.* 
		FROM users
		INNER JOIN uc_addresses_defaults ON uc_addresses_defaults.uid = users.uid
		INNER JOIN uc_addresses ON uc_addresses.aid = uc_addresses_defaults.aid
		WHERE users.uid = :uid", array(':uid'=>$client->uid))->fetchObject();
	//$address = db_fetch_array($r_address);
	
	// Create Invoice
	$lease_start_str = sprintf('%d/%d/%d', $form_state['values']['lease_start']['month'], $form_state['values']['lease_start']['day'], $form_state['values']['lease_start']['year']);
	$lease_start = new DateTime($lease_start_str);
	$order_total = $form_state['values']['deposit'] + $form_state['values']['rent_price'];

	// Add uc_orders
	$result = db_insert('uc_orders')
			->fields(array(
			'uid' => $uid,
			'order_status' => 'pending',
			'order_total' => $order_total,
			'product_count' => 1,
			'primary_email' => $client->mail,
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
			'billing_first_name' => $address->first_name,
			'billing_last_name' => $address->last_name,
			'billing_phone' => $address->phone,
			'billing_company' => $form_state['values']['company_name'],
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
	
	if(!$result) {
		return;
	}
	$orderid = $result;
	
	// Ad uc_order_products
	// Deposit
	$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1834,
					'model' => 'DEPOSIT',
					'title' => 'Deposit',
					'qty' => 1,
					'cost' => 0,
					'price' => $form_state['values']['deposit'],
					'data' => '',
				))->execute();	
	//Rent
	$data = array("attributes"=>array("Office Number"=>array($form_state['values']['office_no']),
		"Month"=>array($lease_start->format('F'))));	
	
	$result = db_insert('uc_order_products')
				->fields(array(
					'order_id' => $orderid,
					'nid' => 1803,
					'model' => 'OFFICE',
					'title' => 'Office Rent',
					'qty' => 1,
					'cost' => 0,
					'price' => $form_state['values']['rent_price'],
					'data' => serialize($data),
				))->execute();	
	

	// Create Lease
	$GLOBALS['company_name'] = $form_state['values']['company_name'];
	$GLOBALS['lease_date'] = date('F j, Y');
	$GLOBALS['lease_start'] = $lease_start->format('F j, Y');
	$GLOBALS['possession_date'] = $lease_start->format('F j, Y');
	$GLOBALS['suite_no'] = $form_state['values']['office_no'];
	$GLOBALS['rent'] = $form_state['values']['rent_price'];
	$GLOBALS['deposit'] = $form_state['values']['deposit'];
	$GLOBALS['first_name'] = $address->first_name;
	$GLOBALS['last_name'] = $address->last_name;
	$GLOBALS['title'] = $form_state['values']['title'];

//	drupal_set_message('Lease Start: '. print_r($form_state['values']['lease_start'], TRUE));
	// Set Document Name
	switch($form_state['values']['lease_type']) {
		case 'Month To Month' :
			$doc_name = 'Lease-M2M';
			break;
			
		case '6 Month' :
			$doc_name = 'Lease-6M';
			$lease_end = $lease_start;
			$lease_end->modify('+6 Months');
			$GLOBALS['lease_end'] = $lease_end->format('F j, Y');
			break;
		
		case '12 Month' :
			$doc_name = 'Lease-12M';
			$lease_end = $lease_start;
			$lease_end->modify('+12 Months');
			$GLOBALS['lease_end'] = $lease_end->format('F j, Y');
			break;
		
		case '24 Month' :
			$doc_name = 'Lease-24M';
			$lease_end = $lease_start;
			$lease_end->modify('+24 Months');
			$GLOBALS['lease_end'] = $lease_end->format('F j, Y');
			break;
		
	}
	// Open the OpenOffice Template
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');

	$TBS = new clsTinyButStrong;
	$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
	
	//$TBS->LoadTemplate('C:/xampp/htdocs/CMS/wyregne/sites/all/modules/wysos/'.$doc_name.'.odt');
	$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name.'.odt');
	$TBS->Show(TBS_OUTPUT);
	//$TBS->Show(TBS_OUTPUT, $doc_name.'-'.$form_state['values']['company_name'].'.odt');
	//$TBS->Show(OPENTBS_DOWNLOAD, $doc_name.'-'.$form_state['values']['company_name'].'.odt'); 
	// Email Lease and Invoice Link To office manager
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
