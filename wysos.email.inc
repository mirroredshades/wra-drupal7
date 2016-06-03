<?php
function wysos_send_renewal_emails_form($form,&$form_state){
	$results = db_query("SELECT DISTINCT DATE_FORMAT(`due_date`,'%m/%e/%Y') AS due_date FROM wra_renewals WHERE DATEDIFF(CURRENT_DATE(),due_date) < 180")->fetchAll();
	
	if(!$results){
		die('Invalid query');
	}
	$options = array();
	foreach($results as $re){
		$options[] = $re->due_date;
	}
	$form['due_date'] = array(
		'#title' => t('Due Date'),
        '#type' => 'select',
        '#options' => $options,
        '#required' => TRUE
	);
	$form['email_type'] = array(
		'#title' => t('Email Type'),
        '#type' => 'select',
        '#options' => array('initial', 'reminder', 'overdue'),
        '#required' => TRUE
	);
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	return $form; 
}

/*
 * Submit wysos_send_renewal_emails_form()
 *
 */
function wysos_send_renewal_emails_form_submit($form, &$form_state) {
	$sql = "SELECT renewal_id, filing_name, FILING_NUM, next_ar_due, due_date, user_name, password, email, ttotal_due, uid, invoice_no\n"
    . "FROM wra_renewals\n"
    . "WHERE DATE_FORMAT(due_date,'%m/%e/%Y')='" . $form['due_date']['#options'][$form_state['values']['due_date']]."' AND payment_date Is Null "
    . "ORDER BY filing_name";
	$result = db_query($sql)->fetchAll();
	if (!$result) {
		echo 'Form State:<br>';
		print_r($form_state);
		echo '<br>Form:';
		print_r($form);
		echo '<br>';
		die('Invalid query1: ' . $sql);
		
	}  

	//define batch API process.
	$batch = array(
	  'operations' => array(),
	  'finished' => 'wysos_send_renewal_email_finished',
	  'title' => t('Sending Emails'),
	  'init_message' => t('Starting...'),
	  'progress_message' => t('Batch @current out of @total'),
	  'error_message' => t('An error occurred and some or all of the batch has failed.'),
	);
	foreach ($result as $rows){
		$batch['operations'][] = array('wysos_send_renewal_email', array($rows,$form['email_type']['#options'][$form_state['values']['email_type']]));
	}	
	batch_set($batch); 
}

function wysos_send_renewal_email($dbrow,$email_type,&$context) {
	global $user;
	
	switch($email_type) {
		case "initial" :
			$message = array(
			  'to' => $dbrow->email,
			  'subject' => t('Registered agent for '.$dbrow->filing_name.' is due'),
			  'body' => t("Hello,\n\nYour annual report and registered agent service for ".$dbrow->filing_name." is due ".$dbrow->due_date.". You can pay your invoice online with a credit card by going to "
			  			."https://my.wyomingregisteredagent.com/renew/".$dbrow->invoice_no."\n\nYou can also check the status of your business entities, download free corporate resolutions, and manage your " 
						."account online by logging in to our website at: http://my.wyomingregisteredagent.com and using your login information:\n\n"
						."Login: ".$dbrow->user_name."\nPassword: ".$dbrow->password."\n\nThank you for choosing WyomingRegisteredAgent.com, Inc.\n\nSincerely,\nWyomingRegisteredAgent.com, Inc\n"
						."2510 Warren Ave.\nCheyenne, WY 82001\n(307) 637-5151\n"),
			  'headers' => array('From' => 'info@wyomingregisteredagent.com'),
			);
			drupal_mail_send($message); 
			// Add wra_user_notes
			$note = "Initial renewal ".$dbrow->invoice_no." for ".$dbrow->filing_name." emailed to ".$dbrow->email;
			$result = db_insert('wra_user_notes')
					->fields(array(
						'uid' => $dbrow->uid,
						'created_uid' => $user->uid,
						'category' => 'billing',
						'notes' => $note,
						'created' => REQUEST_TIME,
					))
					->execute();
			// Add wra_user_notes
			$result = db_insert('wra_renewal_emails')
					->fields(array(
						'renewal_id' => $dbrow->renewal_id,
						'email' => $dbrow->email,
						'template' => $email_type,
						'sent' => REQUEST_TIME,
					))
					->execute();
			// Update Initial email sent
			$result = db_update('wra_renewals')
					->fields(array(
						'initial_email_sent' => REQUEST_TIME,
					))->condition('renewal_id',$dbrow->renewal_id,'=')
					->execute();
			break;
			
		case "reminder" :
			$message = array(
			  'to' => $dbrow->email,
			  'subject' => t('Reminder: Registered agent for '.$dbrow->filing_name.' is due'),
			  'body' => t("Hello,\n\nYour annual report and registered agent service for ".$dbrow->filing_name." is due ".$dbrow->due_date.". You can pay your invoice online with a credit card by going to "
			  			."https://my.wyomingregisteredagent.com/renew/".$dbrow->invoice_no."\n\nYou can also check the status of your business entities, download free corporate resolutions, and manage your " 
						."account online by logging in to our website at: http://my.wyomingregisteredagent.com and using your login information:\n\n"
						."Login: ".$dbrow->user_name."\nPassword: ".$dbrow->password."\n\nThank you for choosing WyomingRegisteredAgent.com, Inc.\n\nSincerely,\nWyomingRegisteredAgent.com, Inc\n"
						."2510 Warren Ave.\nCheyenne, WY 82001\n(307) 637-5151\n"),
			  'headers' => array('From' => 'info@wyomingregisteredagent.com'),
			);
			drupal_mail_send($message); 
			// Add wra_user_notes
			$note = "Reminder renewal ".$dbrow->invoice_no." for ".$dbrow->filing_name." emailed to ".$dbrow->email;		
			$result = db_insert('wra_user_notes')
					->fields(array(
						'uid' => $dbrow->uid,
						'created_uid' => $user->uid,
						'category' => 'billing',
						'notes' => $note,
						'created' => REQUEST_TIME,
					))->execute();
			// Add wra_user_notes
			$result = db_insert('wra_renewal_emails')
					->fields(array(
						'renewal_id' => $dbrow->renewal_id,
						'email' => $dbrow->email,
						'template' => $email_type,
						'sent' => REQUEST_TIME,
					))
					->execute();
			$result = db_update('wra_renewals')
					->fields(array(
						'reminder_email_sent' => REQUEST_TIME,
					))->condition('renewal_id',$dbrow->renewal_id,'=')
					->execute();
			break;

		case "overdue" :
			$message = array(
			  'to' => $dbrow->email,
			  'subject' => t('Registered agent for '.$dbrow->filing_name.' is overdue'),
			  'body' => t("Hello,\n\nYour annual report and registered agent service for ".$dbrow->filing_name." was due ".$dbrow->due_date.". You still have time to file your annual report without "
						."additional fees and penalties being asessed by the Secretary of State of Wyoming. You can pay your invoice online with a credit card by going to "
			  			."https://my.wyomingregisteredagent.com/renew/".$dbrow->invoice_no."\n\nYou can also check the status of your business entities, download free corporate resolutions, and manage your " 
						."account online by logging in to our website at: http://my.wyomingregisteredagent.com and using your login information:\n\n"
						."Login: ".$dbrow->user_name."\nPassword: ".$dbrow->password."\n\nThank you for choosing WyomingRegisteredAgent.com, Inc.\n\nSincerely,\nWyomingRegisteredAgent.com, Inc\n"
						."2510 Warren Ave.\nCheyenne, WY 82001\n(307) 637-5151\n"),
			  'headers' => array('From' => 'info@wyomingregisteredagent.com'),
			);
			drupal_mail_send($message); 
			// Add wra_user_notes
			$note = "Overdue renewal ".$dbrow['invoice_no']." for ".$dbrow['filing_name']." emailed to ".$dbrow['email'];
			$result = db_insert('wra_user_notes')
					->fields(array(
						'uid' => $dbrow->uid,
						'created_uid' => $user->uid,
						'category' => 'billing',
						'notes' => $note,
						'created' => REQUEST_TIME,
					))->execute();
			
			// Add wra_user_notes
			$result = db_insert('wra_renewal_emails')
					->fields(array(
						'renewal_id' => $dbrow->renewal_id,
						'email' => $dbrow->email,
						'template' => $email_type,
						'sent' => REQUEST_TIME,
					))
					->execute();
			$result = db_update('wra_renewals')
					->fields(array(
						'overdue_email_sent' => REQUEST_TIME,
					))->condition('renewal_id',$dbrow->renewal_id,'=')
					->execute();
			break;
	}
	$context['results'][] = $email_type .' : '. $rows['email'];
	$context['message'] = $rows['email'];
}

function wysos_send_renewal_email_finished($success, $results, $operations) {
	if ($success) {
		$message = format_plural(count($results), 'One email processed.', '@count emails processed.');
		}
	else {
		$message = t('Finished with an error.');
		}
	drupal_set_message($message);
	// Providing data for the redirected page is done through $_SESSION.
	foreach ($results as $result) {
		$items[] = t('Sent email %title.', array('%title' => $result));
		}
	$_SESSION['my_batch_results'] = $items;
}
