<?php 

function wysos_renewal_list_form($form,&$form_state)
{
	$result = db_query("SELECT DISTINCT DATE_FORMAT(`due_date`,'%m/%e/%Y') AS due_date FROM wra_renewals WHERE DATEDIFF(CURRENT_DATE(), due_date) < 180")->fetchAll();
	//$result = db_query($sql);
	if (!$result) {
		die('Invalid query: ' . $result);
	}  

	$options = array();
	foreach ($result as $rows) {
		$options[$rows->due_date] = $rows->due_date;
	}

	$form['due_date'] = array(
		'#title' => t('Due Date'),
        '#type' => 'select',
        '#options' => $options,
        //'#default_value' => $options['0'],
        '#required' => TRUE
	);
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	return $form; 
}
function wysos_renewal_list_form_submit($form, &$form_state)
{
	sscanf($form_state['values']['due_date'], "%d/%d/%d", $month, $day, $year);
	drupal_goto('wysos/renewal/list/'.$month.'/'.$year);
}
function wysos_renewal_list($month, $year)
{
	setlocale(LC_MONETARY, 'en_US');
	$date = sprintf('%d-%d-01',$year,$month);
	$result = db_select('wra_renewals','wr');
		$result->join('uc_addresses_defaults','uad','uad.uid = wr.uid');
		$result->join('uc_addresses','ua', 'ua.aid = uad.aid');
		$result->isNull('wr.payment_date');
		$result->condition('wr.due_date',$date,'=');
		$result->fields('wr',array('filing_name','uid','FILING_NUM','last_ar','first_name', 'last_name', 'total_due', 'invoice_no'));
		$result->fields('ua',array('phone'));
		$result->orderBy('wr.invoice_no');
		$result = $result->execute()->fetchAll();

	    $header = array( 'Renewal', 'Company', 'Customer', 'Phone', 'Total', 'Options'  );

	foreach($result as $rows)
	{
		$row[]  = array(
			'style' => 'background-color:#EEE;',
			'data'  => 	array(
				array('data' => l($rows->invoice_no, 'renew/' . $rows->invoice_no)),
				array('data' => $rows->filing_name),
				array('data' => l($rows->first_name.' '.$rows->last_name , 'wysos/user/'.$rows->uid, array('attributes' => array('target' => '_user')))),
				array('data' => $rows->phone),
				array('data' => $rows->total_due),
				array('data' => '')));
	};
	$output = theme('table', array('header'=>$header, 'rows'=>$row));
	
	return $output;
}


