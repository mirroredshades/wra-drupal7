<?php
function wysos_affiliate_mf_summary() {
	drupal_set_title('Affiliate Mail Forwarding (MF)');
	$header = array( 'Company', 'User', 'Addresses', 'Companies', 'Last Order', 'Balance', 'Options');

	$result = db_query("SELECT am.affiliate_mf_id, am.uid, am.name AS company_name, u.name AS user_name, (SELECT COUNT(*) FROM wra_suites s1 WHERE s1.uid=am.uid AND s1.status='active') AS num_addresses, (SELECT COUNT(*) FROM wra_companies c1 WHERE c1.suite_id IN (SELECT suite_id FROM wra_suites WHERE uid=am.uid AND status='active')) AS num_companies, IFNULL((SELECT SUM(order_total) FROM uc_orders WHERE uid=am.uid) -
IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id IN(SELECT order_id FROM uc_orders WHERE uid=am.uid)),0),0) AS user_balance, terms
			FROM wra_affiliate_mf am
			INNER JOIN users u ON am.uid=u.uid")->fetchAll();
	
	$row = array();
	foreach($result as $mfRow) {
		$options = '<font size=-3>';
		if(!empty($mfRow->terms)) {
			$options .= '<span class="qtip-link">
				<div class="qtip-tooltip">'. $mfRow->terms.'</div>
				Terms 
			  </span>';
			$options .= l('List', 'wysos/affiliate_mf/list/'.$mfRow->affiliate_mf_id) . ' ';
			switch($mfRow->affiliate_mf_id) {
				case 5 : /* Registered Agents of Wyoming */
					$options .= l('Create_Invoice', 'wysos/affiliate_mf/create_invoice/'.$mfRow->affiliate_mf_id).' ';
					break;
					
				default:
					$options .= l('Add_Client', 'wysos/affiliate_mf/add/'.$mfRow->affiliate_mf_id).' ';
					break;
			}

		}
		$options .= '</font>';
		$row[]  = array(
			'style' => 'background-color: LightGrey;',
			'data'  => 	array(
				array('data' => $mfRow->company_name),
				array('data' => l($mfRow->user_name,'wysos/user/'.$mfRow->uid)),
				array('data' => $mfRow->num_addresses),
				array('data' => $mfRow->num_companies),
				array('data' => isset($mfRow->last_order)?$mfRow->last_order:''),
				array('data' => uc_currency_format($mfRow->user_balance)),
				array('data' => $options)));
	} 
	//$output = theme('table', $header, $row);
	$output = array('#markup'=>theme('table',array('header'=>$header,'rows'=>$row)));
	return($output);			
	
}
function wysos_affiliate_mf_list_form($form, &$form_state, $affiliate_mf_id) {
}

function wysos_affiliate_mf_list_form_validate($form, &$form_state) {
}

function wysos_affiliate_mf_list_form_submit($form, &$form_state) {
}
function wysos_affiliate_mf_add_form($form, &$form_state, $affiliate_mf_id) {
	include_once('wysos.util.inc');

	//$result = db_query("SELECT * FROM wra_affiliate_mf WHERE affiliate_mf_id=%d", $affiliate_mf_id);
	$rowMf = db_select('wra_affiliate_mf','wam')
				->fields('wam')
				->condition('affiliate_mf_id',$affiliate_mf_id,'=')
				->execute()->fetchAll();
	//$rowMf = db_fetch_array($result);
	$mf_name = isset($rowMf->name)?$rowMf->name:'';
	drupal_set_title("Add Mail Forwarding Client for ". $mf_name);
	$form['affiliate_mf_id'] = array(
			'#type' => 'value', 
			'#value' => $affiliate_mf_id
		);
	switch($affiliate_mf_id) {
		case 5 : /* Registered Agents of Wyoming */
			$form['company_name'] = array(
				'#title' => t('Company'),
				'#type' => 'textfield',
				'#size' => '50',
				'#maxlength' => '80',
				'#autocomplete_path' => 'wysos/autocomplete/all_companies',
				'#default_value' => isset($form_state['values']['company_name'])?$form_state['values']['company_name']:'',
			);
			$form['suite_no'] = array(
				'#title' => t('Suite No.'),
				'#type' => 'select',
				'#options' => array( 0 => '', 63 => 63, 80 => 80 )
			);
			break;
			
		default: 
			$form['company_name'] = array(
				'#title' => t('Company'),
				'#type' => 'textfield',
				'#size' => '50',
				'#maxlength' => '80',
				'#autocomplete_path' => 'wysos/autocomplete/all_companies',
				'#default_value' => isset($form_state['values']['company_name'])?$form_state['values']['company_name']:'',
			);
			$form['email'] = array(
				'#title' => t("Email"),
				'#type' => 'textfield',
				'#size' => '40',
				'#default_value' => isset($form_state['values']['email'])?$form_state['values']['email']:'',
			);
			$form['first_name'] = array(
				'#title' => t('Customer First Name'),
				'#type' => 'textfield',
				'#size' => '40',
				'#maxlength' => '254',
				'#default_value' => isset($form_state['values']['first_name'])?$form_state['values']['first_name']:'',
			);
			$form['last_name'] = array(
				'#title' => t('Customer Last Name'),
				'#type' => 'textfield',
				'#size' => '40',
				'#maxlength' => '254',
				'#default_value' => isset($form_state['values']['last_name'])?$form_state['values']['last_name']:'',
			);
			$form['street1'] = array(
				'#title' => t('Address'),
				'#type' => 'textfield',
				'#size' => '60',
				'#maxlength' => '254',
				'#default_value' => isset($form_state['values']['street1'])?$form_state['values']['street1']:'',
			);
			$form['street2'] = array(
				'#type' => 'textfield',
				'#size' => '60',
				'#maxlength' => '254',
				'#default_value' => isset($form_state['values']['street2'])?$form_state['values']['street2']:'',
			);
			$form['city'] = array(
				'#title' => t('City'),
				'#type' => 'textfield',
				'#size' => '40',
				'#maxlength' => '254',
				'#default_value' => isset($form_state['values']['city'])?$form_state['values']['city']:'',
			);
			//$form['country'] = uc_country_select(uc_get_field_name('country'),$form_state['values']['country'],NULL,NULL,FALSE);
			//$form['zone'] = uc_zone_select(uc_get_field_name('zone'), $form_state['values']['zone'],NULL, $form_state['values']['country'],NULL,FALSE);
			
			$countries = db_query("SELECT country_id,country_name FROM uc_countries")->fetchAll();
			$country_list = array(); 
			foreach($countries as $country){
				$country_list[$country->country_id] = $country->country_name;
			}
			$form['country'] = array(
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
			$form['zone'] = array(
				'#type' => 'select',
				'#options' => $zone_list,
				'#title' => t('State/Province'),
				'#id' => 'tzone', 
			);
			$form['payment']['zone_load'] = array(
				'#type' => 'item',
			);
			$form['postal_code'] = array(
				'#title' => t('Zip/Postal Code'),
				'#type' => 'textfield',
				'#size' => '40',
				'#maxlength' => '40',
				'#default_value' => isset($form_state['values']['postal_code'])?$form_state['values']['postal_code']:'',
			);
			$form['phone'] = array(
				'#title' => t('Phone Number'),
				'#type' => 'textfield',
				'#size' => '20',
				'#maxlength' => '20',
				'#default_value' => isset($form_state['values']['phone'])?$form_state['values']['phone']:'',
			);
			break;
	}
	
	$form['actions-submit'] = array(
		'#type' => 'submit',
		'#value' => 'Submit'
	);
	
	return $form;
}

function wysos_affiliate_mf_add_form_validate($form, &$form_state) {
}

function wysos_affiliate_mf_add_form_submit($form, &$form_state) {
	switch($form_state['values']['affiliate_mf_id']) {
		case 5 : /* Registered Agents of Wyoming */
				
			break;
			
		default :
			break;
	}
}
function wysos_affiliate_mf_create_invoice($affiliate_mf_id) {
	include_once('wysos.util.inc');
	//$result = db_query("SELECT * FROM wra_affiliate_mf WHERE affiliate_mf_id=%d", $affiliate_mf_id);
	//$rowMf = db_fetch_array($result);
	
	$rowMf = db_select('wra_affiliate_mf','wam')
				->fields('wam')
				->condition('affiliate_mf_id',$affiliate_mf_id,'=')
				->execute()->fetchObject();			
				
	$order_id ='';
	switch($affiliate_mf_id) {
		case 5 : /* Registered Agents of Wyoming */
			$order_id = wysos_util_create_order_from_uid($rowMf->uid);
			// Count Suite 63
//			$result2 = db_query("SELECT filing_num, company_name FROM wra_companies WHERE mail_addr1 LIKE '109 E%%17%%63%%' AND mail_postal_code LIKE '8200%' AND sos_status IN('Active','Delinquent - Tax') ORDER BY company_name");
			$result2 = db_query("SELECT filing_num, company_name FROM wra_companies WHERE mail_addr1 LIKE '109 E%%17%%63%%' AND mail_postal_code LIKE '8200%' AND sos_status IN('Active','Delinquent - Tax') AND filing_date < '2014-10-01' ORDER BY company_name");
			$i = 0;
			$company_list = '';
			//while($row = db_fetch_array($result2)) {
				
			foreach ($result2 as $row){	
				
				
				$i++;
				$company_list = $row->filing_num.' '. $row->company_name.";";
			}
			//print_r($company_list);
			$price = $i * 0.75;
			$data = array('attributes' => array(
				'Street Address' => '109 E. 17th St.',
				'Virtual Suite Number' => '63',
//				'Month' => array(date("F", strtotime("-1 month"))),
				'Month' => array('October'),
				'Year' => array(date("Y", strtotime("-1 month"))),
				'Company List' => $company_list
				));
		//	db_query("INSERT INTO uc_order_products (order_id, nid, title, model, qty, price, data)
			//	VALUES(%d, 1882, 'Affiliate Mail Forwarding (Monthly)', 'AMF', 1, %f, '%s')", $order_id, $price, serialize($data));
				
				db_insert('uc_order_products')
				->fields(array(
				'order_id'=>$order_id,
				'nid'=>'1882',
				'title'=>'Affiliate Mail Forwarding (Monthly)',
				'model'=>'AMF',
				'qty'=> '1',
				'price'=>$price,
				'data'=>serialize($data),
				))->execute();
			$total = $price;
					
			// Count Suite 80
			$result3 = db_query("SELECT filing_num, company_name FROM wra_companies WHERE mail_addr1 LIKE '109 E%%17%%80%%' AND mail_postal_code LIKE '8200%' AND sos_status IN('Active','Delinquent - Tax') AND filing_date < '2014-10-01' ORDER BY company_name");
			$i = 0;
			$company_list = '';
			//while($row = db_fetch_array($result3)) {
			foreach ($result3 as $row){		
				
				$i++;
				$company_list = $row->filing_num . ' '. $row->company_name."; ";
			}
			$price = $i * 0.75;
			$data = array('attributes' => array(
				'Street Address' => '109 E. 17th St.',
				'Virtual Suite Number' => '80',
//				'Month' => array(date("F", strtotime("-1 month"))),
				'Month' => array('October'),
				'Year' => array(date("Y", strtotime("-1 month"))),
				'Company List' => $company_list
				));
		//	db_query("INSERT INTO uc_order_products (order_id, nid, title, model, qty, price, data)
		//		VALUES(%d, 1882, 'Affiliate Mail Forwarding (Monthly)', 'AMF', 1, %f, '%s')", $order_id, $price, serialize($data));
				
				db_insert('uc_order_products')
				->fields(array(
				'order_id'=>$order_id,
				'nid'=>'1882',
				'title'=>'Affiliate Mail Forwarding (Monthly)',
				'model'=>'AMF',
				'qty'=> '1',
				'price'=>$price,
				'data'=>serialize($data),
				))->execute();

			$total += $price;
			//db_query("UPDATE uc_orders SET order_total=%f WHERE order_id=%d", $total, $order_id);
			
			db_update('uc_orders')
			->fields(array(
			'order_total'=>$total))
			->condition('order_id',$order_id, '=')->execute();
			
			
//			unset($_REQUEST['destination']);
//			drupal_goto('https://my.wyomingregisteredagent.com/admin/store/orders/'.$order_id.'/invoice');
			break;
			
		default :
			break;
	}
	
	$output = '<p>Done!</p><p>View Order '.l($order_id, 'admin/store/orders/'.$order_id.'/invoice').'</p>';
	return $output;
}
