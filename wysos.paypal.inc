<?php

function wysos_paypal_import_form($form,&$form_state){
	$form['#attributes'] = array('enctype' => "multipart/form-data");
	$form['active'] = array(
		'#type' => 'file',
		'#title' => t('Active Paypal Report'),
		'#size' => 40,
	);
	$form['inactive'] = array(
		'#type' => 'file',
		'#title' => t('Inactive Paypal Report'),
		'#size' => 40,
	);
	$form['suspended'] = array(
		'#type' => 'file',
		'#title' => t('Suspended Paypal Report'),
		'#size' => 40,
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Process Paypal'),
	
	);
	return $form;
}
function wysos_paypal_import_form_validate($form,&$form_state){
	$fieldName = 'active';
	if(!isset($_FILES['files']) && !is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])){
		form_set_error($fieldName, 'Paypal Report missing for upload.');
	}
	$fieldName = 'inactive';
	if(!isset($_FILES['files']) && !is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])){
		form_set_error($fieldName, 'Paypal Report missing for upload.');
	}
	$fieldName = 'suspended';
	if(!isset($_FILES['files']) && !is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])){
		form_set_error($fieldName, 'Paypal Report missing for upload.');
	}
}

function wysos_paypal_import_form_submit($form, $form_state){
	$fieldName = 'active';
	if(isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])){
		//attempt to save the uploaded file
		//$file = file_save_upload($fieldName, array(), 'uploads/', TRUE);
		$active_file = file_save_upload($fieldName);
		drupal_set_message('active_file:'.print_r($active_file, true));
		//set error if file was not uploaded
		if(!active_file){
			form_set_error($fieldName, 'Error uploading file.');
			return;
		}
		else{
			drupal_set_message('Upload Saved');
		}	
	}
	$fieldName = 'inactive';
	if(isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])){
		//attempt to save the uploaded file
		//$file = file_save_upload($fieldName, array(), 'uploads/', TRUE);
		$active_file = file_save_upload($fieldName);
		drupal_set_message('active_file:'.print_r($active_file, TRUE));
		//set error if file was not uploaded
		if(!active_file){
			form_set_error($fieldName, 'Error uploading file.');
			return;
		}
		else{
			drupal_set_message('Upload Saved');
		}
	
	}
	$fieldName = 'suspended';
	if(isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])){
		//attempt to save the uploaded file
		//$file = file_save_upload($fieldName, array(), 'uploads/', TRUE);
		$active_file = file_save_upload($fieldName);
		drupal_set_message('active_file:'.print_r($active_file, TRUE));
		//set error if file was not uploaded
		if(!active_file){
			form_set_error($fieldName, 'Error uploading file.');
			return;
		}
		else{
			drupal_set_message('Upload Saved');
		}	
	}
	// Get next batch_id
	$batch_id = db_query("SELECT MAX(batch_id)+1 FROM wra_paypal_import");
	// Convert to UTF-8
	$active_outfile = '/tmp/RecurringPayments-Active-utf8.CSV';
	$shell = 'iconv -f UCS-2BE -t UTF-8 -o '.$active_outfile.' "'.$active_file->destination.'"';
	drupal_set_message('shell: '.$shell);
	$output = shell_exec($shell);
	$inactive_outfile = '/tmp/RecurringPayments-Inactive-utf8.CSV';
	$shell = 'iconv -f UCS-2BE -t UTF-8 -o '.$inactive_outfile.' "'.$inactive_file->destination.'"';
	drupal_set_message('shell: '.$shell);
	$output = shell_exec($shell);
	$suspended_outfile = '/tmp/RecurringPayments-Suspended-utf8.CSV';
	$shell = 'iconv -f UCS-2BE -t UTF-8 -o '.$suspended_outfile.' "'.$suspended_file->destination.'"';
	drupal_set_message('shell: '.$shell);
	$output = shell_exec($shell);
	
	// Import the files
	wysos_paypal_import_csv($active_outfile, $batch_id);
	wysos_paypal_import_csv($inactive_outfile, $batch_id);
	wysos_paypal_import_csv($suspended_outfile, $batch_id);

	drupal_set_message('Imported Batch '.$batch_id);
	drupal_goto('wysos/paypal/reconcile/'.$batch_id);
}
function wysos_paypal_import_csv($csv_filename, $batch_id) {
		if (($handle = fopen($csv_filename,'r')) !== FALSE) {
		// Ger Type of Profiles
		$report_type = fgetcsv($handle, 1000, ",");
		$field_name = fgetcsv($handle, 1000, ",");
		$holder = fgetcsv($handle, 1000, ",");
		
		while (($data = fgetcsv($handle, 8000, ",")) !== FALSE) {
			sscanf($data[4], '%f USD', $amount_last_paid);
			sscanf($data[5], '%f USD', $outstanding_balance);
			mb_internal_encoding("utf8");
			$month = substr($data[6],0,2);
			mysql_set_charset("utf8");
			$day = substr($data[6], 3, 2);
			$year = substr($data[6], 6, 4);
			$hour = substr($data[6], 11, 2);
			$minute = substr($data[6], 14, 2);
			$second = substr($data[6], 17, 2);
			$mysql_dt = sprintf('%04d-%02d-%02d %02d:%02d:%02d', intval($year), intval($month), $day, $hour, $minute, $second);
			$mysql_dt = $year.'-'.$month.'-'.$day.' '.$hour.':'.$minute.':'.$second;
			$query = sprintf("INSERT INTO wra_paypal_import (batch_id, import_date, profile_id, description, payer_name, status, amount_last_paid, outstanding_balance, 
				next_bill_date)
				VALUES(%d, NOW(), '%s', '%s', '%s', '%s', '%s', '%s', '%s')", $batch_id, $data[0], $data[1], $data[2], $data[3], $amount_last_paid, $outstanding_balance, $mysql_dt);
//			drupal_set_message("query: ".$query);
			$result = db_query($query);
			$num = count($data);
			$row++;
		}
    	fclose($handle); 
	}
}
function wysos_paypal_reconcile_form($form, &$form_state, $batch_id) {
	$header = array(
		array('data' => t('Action')),
		array('data' => t('Message')),
		array('data' => t('Profile ID'), 'field'=> 'profile_id'),
        array('data' => t('Description'), 'field' => 'description'),
        array('data' => t('Payer Name'), 'field' => 'payer_name'),
        array('data' => t('Status'), 'field' => 'import_status', 'sort' => 'asc'),
        array('data' => t('Amount Last Paid'), 'field' => 'amount_last_paid'),
        array('data' => t('Outstanding Balance'),'field' => 'outstanding_balance'),
        array('data' => t('Next Bill Date'), 'field' => 'next_bill_date')
	);

	$query = sprintf("SELECT i.import_id, i.import_date, i.profile_id, i.description, i.payer_name, i.status AS import_status, i.amount_last_paid, i.outstanding_balance, 		
			DATE_FORMAT(i.next_bill_date, '%%c/%%e/%%Y') AS next_bill_date, o.uid, (SELECT COUNT(*) FROM wra_suites WHERE uid=o.uid) AS suite_count, s.*
		FROM wra_paypal_import i
		LEFT JOIN uc_orders o ON SUBSTRING_INDEX(SUBSTR(i.description, 7), ' ',1) = o.order_id
		LEFT JOIN wra_suites s ON s.suite_id=(SELECT MAX(suite_id) FROM wra_suites WHERE uid=o.uid)
		WHERE batch_id=%d", $batch_id);
//	drupal_set_message('query: '.$query);		
//	drupal_set_message('tablesort: '. tablesort_sql($header));
	$result = db_query($query)->fetchAll();
	
	
	
	$form_state['storage']['pp_import'] = array(array());
	$pp_import = array(array());
	foreach($result as $dbrow) {
		$pp_import[$dbrow->import_id] = $dbrow;	
		$messages = array();
		$color = '';
		// See if order exists
		if(strncasecmp('Order ', $dbrow->description, 6) == 0) {
			sscanf($dbrow->description, 'Order %i at %s', $order_id, $brand);
			if(db_query('SELECT COUNT(*) FROM uc_orders WHERE order_id='.$order_id)->fetchField() != 1) {
				$description = $dbrow->description;
				$payer_name = $dbrow->payer_name;
				$messages[] = 'Order does not exist ';
				$order_id = 0;
				$color = 'red';
			} else {
				$description = 'Order '.l($order_id, 'admin/store/orders/'.$order_id).' at '.$brand;
				$payer_name = l($dbrow->payer_name, 'wysos/user/'.$dbrow->uid);
			}
		} else {
			$description = $dbrow->description;
			$payer_name = $dbrow->payer_name;
		}

		if($dbrow->import_status == 'Inactive' OR $dbrow->import_status == 'Suspended') {
			$color = 'red';
			if($dbrow->suite_id) 
				$options .= l('Email_Signup', 'wysos/virtualoffice/email_signup/'.$dbrow->suite_id, array('attributes' => array('target' => '_email'))).' ';
		} else if($dbrow->suite_count == 1 AND $dbrow->import_status == 'Active' AND $color != 'red') {
			$color = 'green';
		} 
		if($dbrow->suite_count == 1 && $dbrow->suite_id > 0) {
			// Update suite record
		} else {
			if($color != 'red')
				$color = 'yellow';
			$messages[] .= 'Suite count is '.$dbrow->suite_count.' ';
		}
		$message = '';
		foreach ($messages as $value) {
			$message .= $value.'<br>';
		}
		
	$form_state['storage']['pp_import'] = $pp_import;
		if($dbrow->suite_id) 
			$options .= l('Cancel', 'wysos/virtualoffice/cancel/'.$dbrow->suite_id, array('attributes' => array('target' => '_email'))).' ';
		$options = wysos_paypal_reconcile_options($dbrow, $messages);
		$rows[]  = array(
				'style' => 'background-color: '.$color.'; font-size: 11px;',
				'data'  => 	array(
					array('data' => $options),
					array('data' => $message),
					array('data' => $dbrow->profile_id),
					array('data' => $description),
					array('data' => $payer_name),
					array('data' => $dbrow->import_status),
					array('data' => $dbrow->amount_last_paid),
					array('data' => $dbrow->outstanding_balance),
					array('data' => $dbrow->next_bill_date),
					));
					
		} 

	$form['table'] = array('#markup'=>theme('table', array('header'=>$header,'rows' => $rows)));	
	//$form['day_'.$row->anniv]['list'] = array('#markup'=>theme('table', array('header'=>$header,'rows' => $day_row)));
	
	$form['list_wrapper'] = array (
//		'#type' => 'item',
//		'#tree' => FALSE,
		'#prefix' => '<br /><div id="list-wrapper">',
		'#value' => $form['table'],
		'#suffix' => '</div>',
	);
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => 'Reconcile'
		);
	
	return $form;
	
}
function wysos_paypal_reconcile_form_validate($form, $form_state) {
	
}

function wysos_paypal_reconcile_form_submit($form, $form_state) {
//	drupal_set_message('$form_state: '.print_r($form_state, true));	
//	drupal_set_message('$clicked_button: '.print_r($form_state['clicked_button'], true));	
//	drupal_set_message('$post: '.print_r($form_state['clicked_button']['#post'], true));	

	foreach($form_state['input'] as $key => $value) {
//drupal_set_message('key: '.$key.'  value: '.$value);
		$import_id = substr(strrchr($key, '$'),1);
//		drupal_set_message('import_id: '.$import_id.' row: '.print_r($row,true));
		if($import_id) {
			$dbrow = $form_state['storage']['pp_import'][$import_id];
			switch($value) {
				case 'update_suite' :
					wysos_paypal_update_suite($dbrow);
					break;
					
				case 'send_email' :
					wysos_paypal_email_suite($dbrow);
					break;

				case 'cancel' 	:
					wysos_paypal_cancel_suite($dbrow);
					break;

			};
		}
	}
}
function wysos_paypal_reconcile_options($row) {

	$options[''] = '(Do Nothing)';
	$default_value = '';
	if(empty($row->suite_id) || $row->suite_count > 1) {

	} else {
		// Suite ID Matched
		if($row->pay_by != 'paypal_wpp' OR trim($row->paypal_profile_id) != trim($row->profile_id) OR strcasecmp($row->import_status, $row->status) != 0) {
			$options['update_suite'] = 'Update VO';
			$default_value = 'update_suite';
		}
	}
	if(!empty($row->suite_id)) {
		if(strtolower($row->import_status) == 'inactive' || strtolower($row->import_status) == 'suspended') {
			$options['send_email'] = 'Email Signup';	
		}
	}

	$options['cancel'] = 'Cancel';	
	$form[] = array(
		'#type' => 'select',
		'#name' => 'action$'.$row->import_id,
		'#options' => $options,
		'#default_value' => $default_value
//		'#default_value' => 'update_suite'
		);

 	return drupal_render($form);
}
function wysos_paypal_update_suite($row) {
//	drupal_set_message('update_suite '.$row->import_id);
//	drupal_set_message('update_suite '.print_r($row));
	
	$query = sprintf("UPDATE wra_suites SET pay_by='paypal_wpp', paypal_profile_id='%s',status='%s',last_status_update=%u WHERE suite_id=%d", $row->profile_id, 
		strtolower($row->import_status), time(), $row->suite_id);
	$result = db_query($query); 
	drupal_set_message('wysos_paypal_update_suite query: '.$query);

};

function wysos_paypal_email_suite($row) {
	drupal_set_message('email_suite '.$row->import_id);
	wysos_paypal_email_for_signup($row->suite_id)	;
};

function wysos_paypal_cancel_suite($row) {
	drupal_set_message('cancel_suite '.$row->import_id);
	
};
function wysos_paypal_vterminal_form($form,&$form_state) {
	$form['order_id'] = array(
	'#type' => 'textfield',
	'#title' => t('Order #'),
	'#size' => 6,
	'#maxlength' => 6,
	);
	$form['amount'] = array(
	'#type' => 'textfield',
	'#title' => t('Payment Amount'),
	'#size' => 8,
	'#maxlength' => 8,
	);
	if (variable_get('uc_credit_type_enabled', FALSE)) {
		$types = variable_get('uc_credit_accepted_types', implode("\r\n", array(t('Visa'), t('Mastercard'), t('Discover'), t('American Express'))));
		if (empty($types)) {
		  $types = array(t('N/A'));
		}
		else {
		  $types = explode("\r\n", $types);
		}
		foreach ($types as $type) {
		  $options[check_plain($type)] = $type;
		}
		$form['payment']['cc_type'] = array(
		  '#type' => 'select',
		  '#title' => t('Card type'),
		  '#options' => $options,
		 // '#default_value' => $_POST['cc_type'],
		);
	} 
	$form['payment']['cc_number'] = array(
	'#type' => 'textfield',
	'#title' => t('Card number'),
//	'#default_value' => $_SESSION['clear_cc'] ? '' : $default_num,
	'#attributes' => array('autocomplete' => 'off'),
	'#size' => 20,
	'#maxlength' => 19,
	);
	
	if (variable_get('uc_credit_start_enabled', FALSE)) {
	$form['payment']['cc_start_month'] = uc_select_month(t('Start Month'), $_POST['cc_start_month'], TRUE);
	$form['payment']['cc_start_year'] = uc_select_year(t('Start Year'), $_POST['cc_start_year'], date('Y') - 10, date('Y'), TRUE);
	}
	
	$form['payment']['cc_exp_month'] = uc_select_month(t('Expiration Month'));
	$form['payment']['cc_exp_year'] = uc_select_year(t('Expiration Year'));
	
	if (variable_get('uc_credit_cvv_enabled', TRUE)) {
		$form['payment']['cc_cvv'] = array(
		  '#type' => 'textfield',
		  '#title' => t('CVV'),
		  '#default_value' => $_SESSION['clear_cc'] ? '' : $default_cvv,
		  '#attributes' => array('autocomplete' => 'off'),
		  '#size' => variable_get('uc_credit_amex', TRUE) ? 4 : 3,
		  '#maxlength' => variable_get('uc_credit_amex', TRUE) ? 4 : 3,
		);
	}
    $form['charge_card'] = array(
      '#type' => 'submit',
      '#value' => t('Charge amount'),
    );
	return ($form);

}

function wysos_paypal_vterminal_form_validate($form, &$form_state) {
	if (!is_numeric($form_state['values']['amount']) || $form_state['values']['amount'] <= 0) {
		form_set_error('amount', t('You must enter a positive number for the amount.'));
	}
}

function wysos_paypal_vterminal_form_submit($form, &$form_state) {
  	global $user;

  	$order = uc_order_load($order_id);
	
	// Set Payment to PayPal
	$data= array();
	$data['txn_type'] = UC_CREDIT_AUTH_CAPTURE;
	$order->payment_details = $form_state['values']['payment'];
	uc_order_save($order);
	$result = uc_paypal_wpp_charge($form_state['values']['order_id'], $form_state['values']['amount'], $data);
	
	drupal_set_message($result->message, $result->success ? 'status' : 'error');
}
function wysos_paypal_vterminal() {
	drupal_get_form('uc_credit_terminal_form', $order, FALSE);
}
function wysos_paypal_pay_invoice_form($form,&$form_state) {
	$form['order_id'] = array(
		'#type' => 'textfield',
		'#title' => t('Order #'),
		'#size' => 7,
		'#maxlength' => 7,
	);
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Goto Payment'),
    );
	
	return($form); 
	drupal_set_message('test');
}

function wysos_paypal_pay_invoice_form_validate($form, &$form_state) {
  $order = uc_order_load($form_state['values']['order_id']);
  if ($order === FALSE) {
	form_set_error('order_id', t('Invalid Order #'));
  } 
}

function wysos_paypal_pay_invoice_form_submit($form, &$form_state) {
	$order = uc_order_load($form_state['values']['order_id']);
	drupal_redirect_form($form, 'wysos/paypal/pay_invoice/'.$form_state['values']['order_id']);
}
function wysos_paypal_pay_invoiceno($order_id) {
	drupal_set_message('pay_invoiceno('.$order_id.')');
	module_load_include('inc', 'uc_credit', 'uc_credit.admin');
    //module_load_include('inc', 'uc_payment', 'uc_credit.admin');
	//include_once('./sites/all/modules/ubercart/payment/uc_credit/uc_credit.admin.inc');
	$order = uc_order_load($order_id);
	$output = drupal_get_form('uc_credit_terminal_form', $order);
	return $output;
}