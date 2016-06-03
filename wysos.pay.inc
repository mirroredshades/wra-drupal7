<?php
/*
 * Renew a Wyoming entity
 * Paramaeters are:
 */
 
function wysos_renew_entity(){
	global $user;
	if($user->uid == 0){
		$output = t('Renew a Wyoming Entity'); 
		$output .= drupal_get_form('wysos_renew_entityform');
		return $output;	
	}
	else{
		drupal_goto('/pay');
	}

}
/*
 * Define form to add renew WySos Entity
 *
 */
function wysos_renew_entityform($form, &$form_state) {
	$form['invoiceno'] = array(
		'#title' => t('Invoice No'),
        '#type' => 'textfield',
        '#size' => '7',
		'#maxlength' => '7',
        '#default_value' => '',
        '#required' => TRUE,
		'#description' => t('Please enter invoice number.')
	);
	$form['submit'] = array (
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	return $form; 
}
/*
 * Validate wysos_renew_entityform()
 *
 */ 

function wysos_renew_entityform_validate($form, &$form_state){
	if(! preg_match('/^[Rr][0-9]{6}$/', $form_state['values']['invoiceno'])){
		form_set_error('cid', t('Invaild Invoice Number.'));
	}
	else{
		$query = db_query('SELECT COUNT(*) FROM wra_renewals WHERE invoice_no=\''. strtoupper($form_state['values']['invoiceno']).'\' AND ISNULL(payment_date)')->fetchField();
		if($query == 0){
			form_set_error('invoiceno', t('Invoice Number not found'));
		}
	}
}

/*
 * Submit wysos_renew_entityform()
 *
 */
function wysos_renew_entityform_submit($form, &$form_state){
	$invoiceno = $form_state['values']['invoiceno'];
	drupal_redirect_form($form,'renew/' . $invoiceno);
}

function wysos_renewal_form3($form, &$form_state, $invoiceno = NULL){
	if(empty($form_state['storage']['step'])){
		$form_state['storage']['step'] = 1;
		$result = db_select('wra_renewals','r');
		$result->join('wra_companies','c','c.filing_id = r.FILING_ID');
		$result->fields('c',array('sos_status'));
		$result->fields('r', array('renewal_id', 'filing_name', 'FILING_TYPE', 'FILING_NUM', 'due_date', 'tdue_date', 'ar_price', 'nominee_price', 'ra_price', 'vo_price', 'total_due', 'ttotal_due', 'Detail1', 'Detail1Price', 'Detail2', 'Detail2Price', 'Detail3', 'Detail3Price', 'Detail4', 'Detail4Price', 'Detail6', 'Detail6Price', 'reinstate_price', 'invoice_no', 'email', 'uid'));
		$result->condition('r.invoice_no', $invoiceno, '=' );
		$result->isNull('r.payment_date');
		$renewal = $result->execute()->fetchObject();
		
		
		if (!$result) {
			return;
		}  
		$form_state['storage']['renewal'] = $renewal;
	}
	
	if($form_state['storage']['renewal']){
	
	if(($form_state['storage']['renewal']->FILING_TYPE == 'Limited Partnership' || $form_state['storage']['renewal']->FILING_TYPE == 'Statutory Trust') && $form_state['storage']['step'] == 2){
		$form_state['storage']['step'] = 3;
	
	}
}
	include_once('wysos.util.inc');
	
	if(isset($renewal)?$renewal:''){
	
	switch($form_state['storage']['step']){
		case 1:
			$renewal = $form_state['storage']['renewal'];
			drupal_set_title('Renew'.$renewal->filing_name. ': Step 1');
			$total = 0;
			setlocale(LC_MONETARY, 'en_US');
			$prefix  = '<table cellspacing=4 cellpadding=4><tr><td>Invoice No:</td><td>'.$invoiceno.'</td><td>Due Date:</td><td>'.$renewal->tdue_date.'</td></tr>';
			$prefix .= '<tr><td>Company:</td><td colspan=4>'.$renewal->filing_name.'</td></tr></table><br><table border=1 cellspacing=2 cellpadding=2><tr><td colspan=4>Invoice Detail</td></tr>';
			
			if (!empty($renewal->Detail1)) {
				$form['ar'] = array(
					'#attributes' => array('onClick' => 'if(ar.checked) { submit_ar.disabled = false; } else { submit_ar.checked = false; submit_ar.disabled = true; };'), 
					'#type' => 'checkbox',
					'#default_value' => isset($form_state['storage']['values'][1]['ar'])?$form_state['storage']['values'][1]['ar']:'',
					'#prefix' => $prefix.'<tr><td width=50>',
					'#suffix' => '</td><td colspan=2>'.$renewal->Detail1.'</td><td>'.money_format('%n',isset($form_state['storage']['ar_tax'])?$form_state['storage']['ar_tax']:$renewal->ar_price).'</td></tr>',
					'#ajax' => array(
						  'path' => 'wysos/renewal_form_calc',
						  'wrapper' => 'total-wrapper',
						  'method' => 'replace',
						  'effect' => 'fade',
						),
					);
				$form['ar_price'] = array(
					'#type' => 'hidden',
		'#value' => isset($form_state['storage']['ar_tax'])?$form_state['storage']['ar_tax']:$renewal->ar_price,
				);
				$prefix = '';
				if($form_state['values']['ar'] || $form_state['storage']['values'][1]['ar'] || !isset($form_state['storage']['values'])) {
					$total += $renewal->ar_price; 
//				drupal_add_js('$("ar").clicked(function () {
//					$("submit_ar").disabled();
//        		})','inline');
				} 
			}
			if (!empty($renewal->Detail2)) {
				$form['ra'] = array(
					'#type' => 'checkbox',
					'#default_value' => true,
					'#disabled' => true,
					'#prefix' => $prefix.'<tr><td width=50>',
					'#suffix' => '</td><td colspan=2>'.$renewal->Detail2.'</td><td>'.$renewal->Detail2Price.'</td></tr>'
				);
				$form['ra_price'] = array(
					'#type' => 'hidden',
					'#value' => $renewal->ra_price
				);
				$prefix = '';
				if($form_state['values']['ra'] || $form_state['storage']['values'][1]['ra'] || !isset($form_state['values']))
					$total += $renewal->ra_price;
			}
			$form['ra_price'] = array(
				'#type' => 'hidden',
				'#value' => $renewal->ra_price,
			);
			if (!empty($renewal->Detail3)) {
				$form['nominee'] = array(
					'#type' => 'checkbox',
					'#default_value' => isset($form_state['storage']['values'][1]['nominee'])?$form_state['storage']['values'][1]['nominee']:'TRUE',
					'#prefix' => $prefix.'<tr><td>',
					'#suffix' => '</td><td colspan=2>'.$renewal->Detail3.'</td><td>'.$renewal->Detail3Price.'</td></tr>',
					'#ajax' => array(
						  'path' => 'wysos/renewal_form_calc',
						  'wrapper' => 'total-wrapper',
						  'method' => 'replace',
						  'effect' => 'fade',
						),
				);
				$form['nominee_price'] = array(
					'#type' => 'hidden',
					'#value' => $renewal->nominee_price,
				);
				$prefix = '';
				if($form_state['values']['nominee'] || $form_state['storage']['values'][1]['nominee'] || !isset($form_state['values'])) 
					$total += $renewal->nominee_price;
			}
			if (!empty($renewal->Detail4)) {
				$form['vo'] = array(
					'#type' => 'checkbox',
					'#default_value' => isset($form_state['storage']['values'][1]['vo'])?$form_state['storage']['values'][1]['vo']:'TRUE',
					'#prefix' => $prefix.'<tr><td>',
					'#suffix' => '</td><td colspan=2>'.$renewal->Detail4.'</td><td>'.$renewal->Detail4Price.'</td></tr>',
					'#ajax' => array(
						  'path' => 'wysos/renewal_form_calc',
						  'wrapper' => 'total-wrapper',
						  'method' => 'replace',
						  'effect' => 'fade',
						),
				);
				$form['vo_price'] = array(
					'#type' => 'hidden',
					'#value' => $renewal->vo_price,
		
				);
				$prefix = '';
				if($form_state['values']['vo'] || $form_state['storage']['values'][1]['vo'] || !isset($form_state['values'])) 
					$total += $renewal->vo_price;
			}
			if (!empty($renewal->Detail6)) {
				$form['reinstate'] = array(
					'#type' => 'checkbox',
					'#default_value' => isset($form_state['storage']['values'][1]['reinstate'])?$form_state['storage']['values'][1]['reinstate']:'',
					'#prefix' => $prefix.'<tr><td>',
					'#suffix' => '</td><td colspan=2>'.$renewal->Detail6.'</td><td>'.$renewal->Detail6Price.'</td></tr>',
					'#ajax' => array(
						  'path' => 'wysos/renewal_form_calc',
						  'wrapper' => 'total-wrapper',
						  'method' => 'replace',
						  'effect' => 'fade',
						),
				);
				$form['reinstate_price'] = array(
					'#type' => 'hidden',
					'#value' => $renewal->reinstate_price,
				);
				$prefix = '';
				if($form_state['values']['reinstate'] || $form_state['storage']['values'][1]['reinstate'] || !isset($form_state['values'])) 
					$total += $renewal->reinstate_price;
			}
			$form['total_wrapper'] = array (
				'#tree' => FALSE,
		//		'#weight' => -4,
		//		'#prefix' => '<div class="clear-block" id="renewal-total-wrapper">',
				'#prefix' => '<div id="renewal-total-wrapper">',
				'#suffix' => '</div>',
			);
			$form['total_wrapper']['total'] = array(
				'#type' => 'hidden',
				'#value' => $total,
				'#prefix' => '<tr><td></td><td colspan=2>Total</td><td><b><div id="total-wrapper">',
				'#suffix' => money_format('%n',$total).'</div></td></tr></table><p>&nbsp;</p>'
			); 
			$form['total'] = array(
				'#type' => 'hidden',
				'#value' => $total,
			); 
			if (!empty($renewal->Detail1)) {
				$form['submit_ar'] = array(
					'#title' => t('Prepare and submit the annual report for me.'),
					'#type' => 'checkbox',
					'#default_value' => isset($form_state['storage']['values'][1]['submit_ar'])?$form_state['storage']['values'][1]['submit_ar']:'',
					'#suffix' => "<div id='submit_ar-wrapper'></div>",
				);
			}
			break;
			
			case 2 : // Annual Report Officers
			drupal_set_title('Renew '.$form_state['storage']['renewal']->filing_name. ': Step 2');
			switch($form_state['storage']['renewal']->FILING_TYPE) 
			{
				case 'Profit Corporation':
						$form['president_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['president_name'])?$form_state['storage']['values'][2]['president_name']:'',
							'#prefix' => '<b>Name & Address of the Current Officers</b> (only 1 is required)<br><table><tr><td width=130>President</td><td>',
							'#suffix' => '</td>'
						);
						$form['president_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['president_address'])?$form_state['storage']['values'][2]['president_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['vp_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['vp_name'])?$form_state['storage']['values'][2]['vp_name']:'',
							'#prefix' => '<tr><td>Vice-President</td><td>',
							'#suffix' => '</td>'
						);
						$form['vp_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['vp_address'])?$form_state['storage']['values'][2]['vp_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['secretary_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['secretary_name'])?$form_state['storage']['values'][2]['secretary_name']:'',
							'#prefix' => '<tr><td>Secretary</td><td>',
							'#suffix' => '</td>'
						);
						$form['secretary_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['secretary_address'])?$form_state['storage']['values'][2]['secretary_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['treasurer_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['treasurer_name'])?$form_state['storage']['values'][2]['treasurer_name']:'',
							'#prefix' => '<tr><td>Treasurer</td><td>',
							'#suffix' => '</td>'
						);
						$form['treasurer_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['treasurer_address'])?$form_state['storage']['values'][2]['treasurer_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr></table>'
						);
						$form['director1_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['director1_name'])?$form_state['storage']['values'][2]['director1_name']:'',
							'#prefix' => '<b>Name & Address of the Current Directors</b> (none are required)<br><table><tr><td width=130>Director</td><td>',
							'#suffix' => '</td>'
						);
						$form['director1_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['director1_address'])?$form_state['storage']['values'][2]['director1_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['director2_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['director2_name'])?$form_state['storage']['values'][2]['director2_name']:'',
							'#prefix' => '<tr><td>Director</td><td>',
							'#suffix' => '</td>'
						);
						$form['director2_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['director2_address'])?$form_state['storage']['values'][2]['director2_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['director3_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['director3_name'])?$form_state['storage']['values'][2]['director3_name']:'',
							'#prefix' => '<tr><td>Director</td><td>',
							'#suffix' => '</td>'
						);
						$form['director3_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['director3_address'])?$form_state['storage']['values'][2]['director3_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr></table>'
						);
					break;
					
					case 'NonProfit Corporation':
						$form['president_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['president_name'])?$form_state['storage']['values'][2]['president_name']:'',
							'#prefix' => '<b>Name & Address of the Current Officers</b> (only 1 is required)<br><table><tr><td width=130>President</td><td>',
							'#suffix' => '</td>'
						);
						$form['president_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['president_address'])?$form_state['storage']['values'][2]['president_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['vp_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['vp_name'])?$form_state['storage']['values'][2]['vp_name']:'',
							'#prefix' => '<tr><td>Vice-President</td><td>',
							'#suffix' => '</td>'
						);
						$form['vp_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['vp_address'])?$form_state['storage']['values'][2]['vp_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['secretary_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['secretary_name'])?$form_state['storage']['values'][2]['secretary_name']:'',
							'#prefix' => '<tr><td>Secretary</td><td>',
							'#suffix' => '</td>'
						);
						$form['secretary_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['secretary_address'])?$form_state['storage']['values'][2]['secretary_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['treasurer_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['treasurer_name'])?$form_state['storage']['values'][2]['treasurer_name']:'',
							'#prefix' => '<tr><td>Treasurer</td><td>',
							'#suffix' => '</td>'
						);
						$form['treasurer_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['treasurer_address'])?$form_state['storage']['values'][2]['treasurer_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr></table>'
						);
						$form['director1_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['director1_name'])?$form_state['storage']['values'][2]['director1_name']:'',
							'#prefix' => '<b>Name & Address of the Current Directors</b> (none are required)<br><table><tr><td width=130>Director</td><td>',
							'#suffix' => '</td>'
						);
						$form['director1_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['director1_address'])?$form_state['storage']['values'][2]['director1_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['director2_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['director2_name'])?$form_state['storage']['values'][2]['director2_name']:'',
							'#prefix' => '<tr><td>Director</td><td>',
							'#suffix' => '</td>'
						);
						$form['director2_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['director2_address'])?$form_state['storage']['values'][2]['director2_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr>'
						);
						$form['director3_name'] = array(
							'#type' => 'textfield',
							'#size' => '20',
							'#maxlength' => '40',
							'#default_value' => isset($form_state['storage']['values'][2]['director3_name'])?$form_state['storage']['values'][2]['director3_name']:'',
							'#prefix' => '<tr><td>Director</td><td>',
							'#suffix' => '</td>'
						);
						$form['director3_address'] = array(
							'#type' => 'textfield',
							'#size' => '40',
							'#maxlength' => '60',
							'#default_value' => isset($form_state['storage']['values'][2]['director3_address'])?$form_state['storage']['values'][2]['director3_address']:'',
							'#prefix' => '<td>',
							'#suffix' => '</td></tr></table>'
						);
					break;
					case 'Limited Partnership':
					break;

				case 'Statutory Trust':
					break;
			}
			break;
		
			case 3: // Annual Report Assets
				drupal_set_title('Renew '.$form_state['storage']['renewal']->filing_name. ': Step 3');
				$form['a'] = array (
					'#type' => 'item',
					'#title' => 'List only company assets that are located in the State of Wyoming'
					);
				$form['cash'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['cash'])?$form_state['storage']['values'][3]['cash']:0.00,
					'#title' => '1. Cash'
				);
				$form['accounts_receivable'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['accounts_receivable'])?$form_state['storage']['values'][3]['accounts_receivable']:0.00,
					'#title' => '2a. Trade Notes and Accounts Receivable'
				);
				$form['bad_debts'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['bad_debts'])?$form_state['storage']['values'][3]['bad_debts']:0.00,
					'#title' => '2b. Subtract Allowance for Bad Debts'
				);
				$form['inventories'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['inventories'])?$form_state['storage']['values'][3]['inventories']:0.00,
					'#title' => '3. Inventories'
				);
				$form['gov_obligations'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['gov_obligations'])?$form_state['storage']['values'][3]['gov_obligations']:0.00,
					'#title' => '4. U.S. Government Obligations'
				);
				$form['tax_exempt_securities'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['tax_exempt_securities'])?$form_state['storage']['values'][3]['tax_exempt_securities']:0.00,
					'#title' => '5. Tax Exempt Securities'
				);
				$form['other_current_assets'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['other_current_assets'])?$form_state['storage']['values'][3]['other_current_assets']:0.00,
					'#title' => '6. Other Current Assets'
				);
				$form['stockholder_loans'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['stockholder_loans'])?$form_state['storage']['values'][3]['stockholder_loans']:0.00,
					'#title' => '7. Loans to Stockholders'
				);
				$form['mortgage_loans'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['mortgage_loans'])?$form_state['storage']['values'][3]['mortgage_loans']:0.00,
					'#title' => '8. Loans Owned'
				);
				$form['other_investments'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['other_investments'])?$form_state['storage']['values'][3]['other_investments']:0.00,
					'#title' => '9. Other Investments'
				);
				$form['building_assets'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['building_assets'])?$form_state['storage']['values'][3]['building_assets']:0.00,
					'#title' => '10. Buildings and Other Depreciable Tangible Assets, real and personal'
				);
				$form['depletable_assets'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['depletable_assets'])?$form_state['storage']['values'][3]['depletable_assets']:0.00,
					'#title' => '11. Depletable Assets'
				);
				$form['land'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['land'])?$form_state['storage']['values'][3]['land']:0.00,
					'#title' => '12. Land'
				);
				$form['intangible_assets'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['intangible_assets'])?$form_state['storage']['values'][3]['intangible_assets']:0.00,
					'#title' => '13. Intangible Assets'
				);
				$form['accumulated_amortization'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['accumulated_amortization'])?$form_state['storage']['values'][3]['accumulated_amortization']:0.00,
					'#title' => '14. Accumulated Amortization'
				);
				$form['other_assets'] = array(
					'#type' => 'textfield',
					'#size' => '20',
					'#maxlength' => '20',
					'#default_value' => isset($form_state['storage']['values'][3]['other_assets'])?$form_state['storage']['values'][3]['other_assets']:0.00,
					'#title' => '14. Other Assets'
				);
				
				break;
				
				case 4: 
				// Confirm Totals
				$renewal = $form_state['storage']['renewal'];
				drupal_set_title('Renew '.$renewal->filing_name. ': Step 4');
				$total = 0;
				setlocale(LC_MONETARY, 'en_US');
				$prefix  = '<table cellspacing=4 cellpadding=4><tr><td>Invoice No:</td><td>'.$renewal->invoice_no.'</td><td>Due Date:</td><td>'.$renewal->tdue_date.'</td></tr>';
				$prefix .= '<tr><td>Company:</td><td colspan=4>'.$renewal->filing_name.'</td></tr></table><br><table border=1 cellspacing=2 cellpadding=2><tr><td colspan=4>Invoice Detail</td></tr>';
				if ($form_state['storage']['values'][1]['ar']) {
					$form['ar'] = array(
						'#type' => 'checkbox',
						'#value' => true,
						'#disabled' => true,
						'#prefix' => $prefix.'<tr><td width=50>',
						'#suffix' => '</td><td colspan=2>'.$renewal->Detail1.'</td><td>'.($form_state['storage']['values'][1]['submit_ar'] ? money_format('%n',$form_state['storage']['ar_tax']) : $renewal->Detail1Price).'</td></tr>',
						);
					$prefix = '';
					if($form_state['storage']['values'][1]['submit_ar'] && isset($form_state['storage']['ar_tax'])) 
						$total += $form_state['storage']['ar_tax'];
					else
						$total += $renewal->ar_price; 
				}
				if ($form_state['storage']['values'][1]['ra']) {
					$form['ra'] = array(
						'#type' => 'checkbox',
						'#default_value' => true,
						'#disabled' => true,
						'#prefix' => $prefix.'<tr><td width=50>',
						'#suffix' => '</td><td colspan=2>'.$renewal->Detail2.'</td><td>'.$renewal->Detail2Price.'</td></tr>'
					);
					$prefix = '';
					$total += $renewal->ra_price;
				}
				if ($form_state['storage']['values'][1]['nominee']) {
					$form['nominee'] = array(
						'#type' => 'checkbox',
						'#default_value' => true,
						'#disabled' => true,
						'#prefix' => $prefix.'<tr><td>',
						'#suffix' => '</td><td colspan=2>'.$renewal->Detail3.'</td><td>'.$renewal->Detail3Price.'</td></tr>',
					);
					$prefix = '';
					$total += $renewal->nominee_price;
				}
				if ($form_state['storage']['values'][1]['vo']) {
					$form['vo'] = array(
						'#type' => 'checkbox',
						'#default_value' => true,
						'#disabled' => true,
						'#prefix' => $prefix.'<tr><td>',
						'#suffix' => '</td><td colspan=2>'.$renewal->Detail4.'</td><td>'.$renewal->Detail4Price.'</td></tr>',
					);
					$prefix = '';
					$total += $renewal->vo_price;
				}
				if ($form_state['storage']['values'][1]['reinstate']) {
					$form['reinstate'] = array(
						'#type' => 'checkbox',
						'#default_value' => true,
						'#disabled' => true,
						'#prefix' => $prefix.'<tr><td>',
						'#suffix' => '</td><td colspan=2>'.$renewal->Detail6.'</td><td>'.$renewal->Detail6Price.'</td></tr>',
					);
					$prefix = '';
					$total += $renewal->reinstate_price;
				}
				$form['total_wrapper'] = array (
					'#tree' => FALSE,
			//		'#weight' => -4,
			//		'#prefix' => '<div class="clear-block" id="renewal-total-wrapper">',
					'#prefix' => '<div id="renewal-total-wrapper">',
					'#suffix' => '</div>',
				);
				$form['total_wrapper']['total'] = array(
					'#type' => 'hidden',
					'#value' => $total,
					'#prefix' => '<tr><td></td><td colspan=2>Total</td><td><b><div id="total-wrapper">',
					'#suffix' => money_format('%n',$total).'</div></td></tr></table><p>&nbsp;</p>'
				); 
				$form_state['storage']['total'] = $total;
				$form['cc_num'] = array(
					'#title' => t('Credit Card Number'),
					'#type' => 'textfield',
					'#size' => '21',
					'#maxlength' => '16',
					'#default_value' => isset($form_state['storage']['values'][4]['cc_num'])?$form_state['storage']['values'][4]['cc_num']:'',
			//        '#required' => TRUE,
					'#prefix' => "<p><b>Payment Information</b>&nbsp;<img src='/sites/all/modules/ubercart/payment/uc_credit/images/visa.gif' style='position: relative; top: 5px;'>
						&nbsp;<img src='/sites/all/modules/ubercart/payment/uc_credit/images/mastercard.gif' style='position: relative; top: 5px;'>
						&nbsp;<img src='/sites/all/modules/ubercart/payment/uc_credit/images/discover.gif' style='position: relative; top: 5px;'>
						&nbsp;<img src='/sites/all/modules/ubercart/payment/uc_credit/images/amex.gif' style='position: relative; top: 5px;'></p>"
				);
				$form['cc_exp_month'] = array(
					'#title' => t('Credit Card Expiration Month'),
					'#type' => 'select',
					'#options' => array(1=>'Jan (1)',2=>'Feb (2)',3=>'Mar (3)',4=>'Apr (4)',5=>'May (5)',6=>'Jun (6)',7=>'Jul (7)',8=>'Aug (8)',9=>'Sep (9)',
						10=>'Oct (10)',11=>'Nov (11)',12=>'Dec (12)'),
					'#default_value' => dv($form_state['storage']['values'][4]['cc_exp_month'],date('n')),
			//        '#required' => TRUE,
				);
				$year = date('Y');
				$form['cc_exp_year'] = array(
					'#title' => t('Credit Card Expiration Year'),
					'#type' => 'select',
					'#options' => array($year=>$year,$year+1=>$year+1,$year+2=>$year+2,$year+3=>$year+3,$year+4=>$year+4,$year+5=>$year+5,$year+6=>$year+6,$year+7=>$year+7,
						$year+8=>$year+8, $year+9=>$year+9, $year+10=>$year+10, $year+11=>$year+11),
					'#default_value' => dv($form_state['storage']['values'][4]['cc_exp_year'],date('Y')),
			//        '#required' => TRUE,
				);
				$form['cc_address'] = array(
					'#title' => t('Cardholder Street Address'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '60',
					'#default_value' => isset($form_state['storage']['values'][4]['cc_address'])?$form_state['storage']['values'][4]['cc_address']:'',
			//        '#required' => TRUE
				);
				$form['cc_zip'] = array(
					'#title' => t('Cardholder Zip/Postal Code'),
					'#type' => 'textfield',
					'#size' => '10',
					'#maxlength' => '10',
					'#default_value' => isset($form_state['storage']['values'][4]['cc_zip'])?$form_state['storage']['values'][4]['cc_zip']:'',
			//        '#required' => TRUE
				);
				break;
		
	}
}
	if($form_state['storage']['step'] > 1) {
		$form['previous'] = array(
			'#type' => 'submit',
			'#value' => '<< Previous' 
			);
	}
	
	$form['next'] = array(
		'#type' => 'submit',
		'#value' => 'Next >>'
		);

	return ($form);
}

function wysos_renewal_form3_submit($form, &$form_state)
{
//drupal_set_message('submit clicked: '.$form_state['clicked_button']['#id']);	
	$form_state['storage']['values'][$form_state['storage']['step']] = $form_state['values'];
 
	if($form_state['clicked_button']['#id'] == 'edit-previous') {
		if($form_state['storage']['step'] == 4) {
			if($form_state['storage']['renewal']->FILING_TYPE == 'Profit Corporation' 
					|| $form_state['storage']['renewal']['FILING_TYPE'] == 'Limited Liability Company'
					|| $form_state['storage']['renewal']['FILING_TYPE'] == 'NonProfit Corporation') {
				if($form_state['storage']['values'][1]['submit_ar']) {
					$form_state['storage']['step'] = 3;
				} else {
					$form_state['storage']['step'] = 1;
				}
			}
		}
		else if($form_state['storage']['step'] == 3 && ($form_state['storage']['renewal']->FILING_TYPE == 'Limited Liability Company' 
			|| $form_state['storage']['values'][1]['nominee'])){
			$form_state['storage']['step'] = 1;
			}
		else {
			$form_state['storage']['step']--;
		}
		
	} elseif($form_state['clicked_button']['#id'] == 'edit-next') {
		if($form_state['storage']['step'] == 1) {
		// Get Total
			$no_ar_total = 0;
			if(isset($form_state['storage']['values'][1]['ra'])?$form_state['storage']['values'][1]['ra']:'')
			{
				$no_ar_total += $form_state['storage']['renewal']->ra_price;
			}
			if(isset($form_state['storage']['values'][1]['nominee'])?$form_state['storage']['values'][1]['nominee']:'')
			{
				$no_ar_total += $form_state['storage']['renewal']->nominee_price;
			}
			if(isset($form_state['storage']['values'][1]['vo'])?$form_state['storage']['values'][1]['vo']:'')
			{
				$no_ar_total += $form_state['storage']['renewal']->vo_price;
			}
			$form_state['storage']['no_ar_total'] = $no_ar_total;
			if(isset($form_state['storage']['values'][1]['ar'])?$form_state['storage']['values'][1]['ar']:'' && !isset($form_state['storage']['values'][1]['submit_ar'])?$form_state['storage']['values'][1]['submit_ar']:'')
				{
				$total = $no_ar_total + $form_state['storage']['renewal']->ar_price;
				}
			else{
				$total = $no_ar_total;}
			$form_state['storage']['total'] = $total;
			if(isset($form_state['storage']['renewal']->FILING_TYPE)?$form_state['storage']['renewal']->FILING_TYPE:''){
			if($form_state['storage']['renewal']->FILING_TYPE == 'Profit Corporation' 
					|| $form_state['storage']['renewal']->FILING_TYPE == 'NonProfit Corporation') {
				if($form_state['storage']['values'][1]['submit_ar']) {
					if($form_state['storage']['values'][1]['nominee'])
					{
						$form_state['storage']['step'] = 3;
					} else {
						$form_state['storage']['step'] = 2;
					}
				} else {
					$form_state['storage']['step'] = 4;
				}
			} else if($form_state['storage']['renewal']->FILING_TYPE == 'Limited Liability Company' || $form_state['storage']['renewal']->FILING_TYPE == 'Limited Partnership')
				if( $form_state['storage']['values'][1]['submit_ar']) {
					$form_state['storage']['step'] = 3;
				} else
					$form_state['storage']['step'] = 4;
				}
		} elseif($form_state['storage']['step'] == 2) {
			$form_state['storage']['step']++;
		} elseif($form_state['storage']['step'] == 3) {
			// Add Total Assets, Total AR Fee
			$assets_total = $form_state['storage']['values'][3]['cash']+$form_state['storage']['values'][3]['accounts_receivable'];
			$assets_total -= $form_state['storage']['values'][3]['bad_debts'];
			$assets_total += $form_state['storage']['values'][3]['inventories'] + $form_state['storage']['values'][3]['gov_obligations'];
			$assets_total += $form_state['storage']['values'][3]['tax_exempt_securities'] + $form_state['storage']['values'][3]['other_current_assets'];
			$assets_total += $form_state['storage']['values'][3]['stockholder_loans'] + $form_state['storage']['values'][3]['mortgage_loans'];
			$assets_total += $form_state['storage']['values'][3]['other_investments'] + $form_state['storage']['values'][3]['building_assets'];
			$assets_total += $form_state['storage']['values'][3]['depletable_assets'] + $form_state['storage']['values'][3]['land'];
			$assets_total += $form_state['storage']['values'][3]['intangible_assets'] - $form_state['storage']['values'][3]['accumulated_amortization'];
			$assets_total += $form_state['storage']['values'][3]['other_assets'];
			$form_state['storage']['assets_total'] = $assets_total;
			if($assets_total > 250000) 
				$ar_tax = $assets_total * .0002;
			else 
				$ar_tax = 50;
			$form_state['storage']['ar_tax'] = $ar_tax;
			$form_state['storage']['step']++;
		} elseif($form_state['storage']['step'] == 4) {
			// Authorize Credit Card
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
			
			include"lphp.php";
			$mylphp=new lphp;
			
			// Load User
			$user = user_load($form_state['storage']['renewal']->uid);
			profile_load_profile($user);
			
			# constants
			$myorder["host"]       = "secure.linkpt.net";
			$myorder["port"]       = "1129";
			$myorder["keyfile"]    = "/home/wyregnet/public_html/my/1257409.pem"; # Change this to the name and location of your certificate file 
			$myorder["configfile"] = "1257409";        # Change this to your store number 
			
			# form data
			$myorder["cardnumber"]    = $form_state['storage']['values'][4]['cc_num'];
			$myorder["cardexpmonth"]  = $form_state['storage']['values'][4]['cc_exp_month'];
			$myorder["cardexpyear"]   = substr($form_state['storage']['values'][4]['cc_exp_year'],-2);
			$myorder["chargetotal"]   = $form_state['storage']['total'];
			$myorder["ordertype"]     = "SALE";
			$myorder["addrnum"]      = substr($form_state['storage']['values'][4]['cc_address'], 0, stripos($form_state['storage']['values'][4]['cc_address'], ' '));   # Required for AVS. If not provided, transactions will downgrade.
			$myorder["zip"]          = $form_state['storage']['values'][4]['cc_zip'];	# Required for AVS. If not provided, transactions will downgrade.
			$myorder["ip"]           = $ip;
			$myorder["ponumber"]		= $form_state['storage']['renewal']['invoice_no'];
			# $myorder["debugging"]	= "true";
		//print_r($myorder);
			
			# Send transaction. Use one of two possible methods  #
//drupal_set_message('Xml: '.	print_r($myorder,true));		
//$result["r_approved"] = "APPROVED";
			$result = $mylphp->curl_process($myorder);  # use curl methods
			
			if ($result["r_approved"] == "APPROVED")    // transaction failed, print the reason
			{		
				$approval_date = date("Y-m-d H:m:s");
				$approval_code = $result["r_code"];
				$fd_ordernum = $result["r_ordernum"];
				$fd_avs	= $result["r_avs"];
				

				$result = db_update('wra_renewals')
					->fields(array('payment_date'=>NOW(),'payment_amount'=>$form_state['storage']['total'], 'fd_ordernum'=>$fd_ordernum, 'approval_code'=>$approval_code))
					->condition('invoice_no',$form_state['storage']['renewal']->invoice_no, '=')
					->execute();	
					
				// Add uc_orders
				$num_products = 0;
				if($form_state['storage']['values'][1]['ar']) {
					$num_products++;		
				}
				if($form_state['storage']['values'][1]['ra']) {
					$num_products++;		
				}
				if($form_state['storage']['values'][1]['vo']) {
					$num_products++;		
				}
				if($form_state['storage']['values'][1]['nominee']) {
					$num_products++;		
				}
				if($form_state['storage']['values'][1]['reinstate']) {
					$num_products++;		
				}
				
				// Get Country Number
				$countryid = db_query("SELECT country_id FROM {uc_countries} WHERE country_name=:country_name",array(':country_name'=>$user->profile_country))->fetchField();
				// Get Zone
				$zoneid = db_query("SELECT zone_id FROM {uc_zones} WHERE zone_country_id=:zcid AND (zone_code=:zc OR zone_name=:zn)",array(':zcid'=>$countryid,':zc'=>$user->profile_state,':zn'=>$user->profile_state))->fetchField();
			
				// Add uc_orders
				$result = db_insert('uc_orders')
						->fields(array(
						'uid' => $form_state['storage']['renewal']->uid,
						'order_status' => 'payment_received',
						'order_total' => $form_state['storage']['values'][4]['total_wrapper']['total'],
						'product_count' => $num_products,
						'primary_email' => $user->mail,
						'delivery_first_name' => $user->profile_first_name,
						'delivery_last_name' => $user->profile_last_name,
						'delivery_phone' => $user->profile_phone,
						'delivery_company' => $form_state['storage']['renewal']->filing_name,
						'delivery_street1' => $user->profile_address1,
						'delivery_street2' => $user->profile_address2,
						'delivery_city' =>  $user->profile_city,
						'delivery_zone' => $zoneid,
						'delivery_postal_code' => $user->profile_zip,
						'delivery_country' => $countryid,
						'billing_first_name' => $user->profile_first_name,
						'billing_last_name' => $user->profile_last_name,
						'billing_phone' => $user->profile_phone,
						'billing_company' => $form_state['storage']['renewal']->filing_name,
						'billing_street1' => $user->profile_address1,
						'billing_street2' => $user->profile_address2,
						'billing_city' => $address->city,
						'billing_zone' => $address->zone,
						'billing_postal_code' => $address->postal_code,
						'billing_country' => $user->profile_city,
						'payment_method' => 'credit',
						'created' => REQUEST_TIME,
						'host' => '',
						))->execute();	
				// Ad uc_order_products
				$orderid = $result;
				if($form_state['storage']['values'][1]['ar']) {
					$result = db_insert('uc_order_products')
						->fields(array(
							'order_id'=> $orderid,
							'nid'=> 6,
							'model'=> 'WYAR',
							'title'=> 'Wyoming Annual Report',
							'qty'=> 1,
							'price'=> $form_state['storage']['renewal']->ar_price,
						))->execute();
				}
				if($form_state['storage']['values'][1]['ra']) {
					// create data parameter
					$data = array("attributes"=>array("Name of the company"=>array($form_state['storage']['renewal']->filing_name)),
						"shippable"=>"1");	
					$result = db_insert('uc_order_products')
						->fields(array(
							'order_id'=> $orderid,
							'nid'=> 6,
							'model'=> 'WY1RR',
							'title'=> 'Wyoming Registered Agent Service Renewal-1 Year',
							'qty'=> 1,
							'price'=> $form_state['storage']['renewal']->ra_price,
							'data' => serialize($data),
						))->execute();
				}
				if($form_state['storage']['values'][1]['vo']) {
					// create data parameter
					$data = array("attributes"=>array("Name of the company"=>array($form_state['storage']['renewal']['filing_name'])),
						"shippable"=>"1");	
					
					$result = db_insert('uc_order_products')
						->fields(array(
							'order_id'=> $orderid,
							'nid'=> 6,
							'model'=> 'WYVO',
							'title'=> 'Wyoming Virtual Office-1 Year',
							'qty'=> 1,
							'price'=> $form_state['storage']['renewal']->vo_price,
							'data' => serialize($data),
						))->execute();
				}
			
				if($form_state['storage']['values'][1]['nominee']) {
					if( $form_state['storage']['renewal']->nominee_price > 199 ) {
						if( $form_state['storage']['renewal']->nominee_price > 999 ) {							
							$result = db_insert('uc_order_products')
							->fields(array(
								'order_id'=> $orderid,
								'nid'=> 1097,
								'model'=> 'NATTY',
								'title'=> 'Attorney Privacy',
								'qty'=> 1,
								'price'=> $form_state['storage']['renewal']->nominee_price,
							))->execute();
							
						} else {
							$result = db_insert('uc_order_products')
							->fields(array(
								'order_id'=> $orderid,
								'nid'=> 1096,
								'model'=> 'NFS',
								'title'=> 'Nominee Privacy',
								'qty'=> 1,
								'price'=> $form_state['storage']['renewal']->nominee_price,
							))->execute();
						}
					} else {
						$result = db_insert('uc_order_products')
							->fields(array(
								'order_id'=> $orderid,
								'nid'=> 5,
								'model'=> 'NPR',
								'title'=> 'Nominee Privacy',
								'qty'=> 1,
								'price'=> $form_state['storage']['renewal']->nominee_price,
							))->execute();
					}
				}
				if($form_state['storage']['values'][1]['reinstate']) {
					// create data parameter
					$data = array("attributes"=>array("Name of the company"=>array($form_state['storage']['renewal']->filing_name)),
						"shippable"=>"1");
					
					$result = db_insert('uc_order_products')
							->fields(array(
								'order_id'=> $orderid,
								'nid'=> 1248,
								'model'=> 'WYRE',
								'title'=> 'Reinstatement',
								'qty'=> 1,
								'price'=> $form_state['storage']['renewal']->reinstate_price,
								'data'=> serialize($data),
							))->execute();
				}
			
				// Add uc_payment_receipt
				$comment = "linkpoint: " . $fd_ordernum;
				
				$result = db_query("INSERT INTO {uc_payment_receipts} (order_id,method,amount,uid,comment,received) VALUES (%d,'Credit card',%f,%d,'%s',%d)", $orderid, 
				$form_state['storage']['total'], $user->uid,$comment,time());
				
				$result = db_insert('uc_payment_receipts')
							->fields(array(
								'order_id'=> $orderid,
								'method'=> 'Credit card',
								'amount'=> $form_state['storage']['total'],
								'uid'=> $user->uid,
								'comment'=> $comment,
								'received'=> time(),
							))->execute();
				
				if (!$result) {
		//			die('Invalid query: ' . $query);
				}
		
				$order = uc_order_load($orderid);
			
				// Add Annual Report Records
				if($form_state['storage']['values'][1]['submit_ar'] && $form_state['storage']['values'][1]['ar']) {
					$result = db_insert('wra_renewals_ar')
						->fields(array(
							'renewal_id' => $form_state['storage']['renewal']->renewal_id,
							'cash' => $form_state['storage']['values'][3]['cash'],
							'accounts_receivable' => $form_state['storage']['values'][3]['accounts_receivable'],
							'bad_debts' => $form_state['storage']['values'][3]['bad_debts'],
							'inventories' => $form_state['storage']['values'][3]['inventories'],
							'gov_obligations' => $form_state['storage']['values'][3]['gov_obligations'],
							'tax_exempt_securities' => $form_state['storage']['values'][3]['tax_exempt_securities'],
							'other_current_assets' => $form_state['storage']['values'][3]['other_current_assets'],
							'stockholder_loans' => $form_state['storage']['values'][3]['stockholder_loans'],
							'mortgage_loans' => $form_state['storage']['values'][3]['mortgage_loans'],
							'other_investments' => $form_state['storage']['values'][3]['other_investments'],
							'building_assets' => $form_state['storage']['values'][3]['building_assets'],
							'depletable_assets' => $form_state['storage']['values'][3]['depletable_assets'],
							'land' => $form_state['storage']['values'][3]['land'],
							'intangible_assets' => $form_state['storage']['values'][3]['intangible_assets'],
							'accumulated_amortization' => $form_state['storage']['values'][3]['accumulated_amortization'],
							'other_assets' => $form_state['storage']['values'][3]['other_assets'],
							'total' =>  $form_state['storage']['assets_total'],
							'tax' => $form_state['storage']['ar_tax']
						))->execute();
					if(($form_state['storage']['renewal']->FILING_TYPE == 'Profit Corporation' || $form_state['storage']['renewal']->FILING_TYPE == 'NonProfit Corporation')
						&& !$form_state['storage']['values'][1]['nominee'])
						{
						if(!empty($form_state['storage']['values'][2]['president_name'])) {
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'President',
								'name'=> $form_state['storage']['values'][2]['president_name'],
								'address'=> $form_state['storage']['values'][2]['president_address'],
							))->execute();							
						}
						if(!empty($form_state['storage']['values'][2]['vp_name'])) {							
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'Vice President',
								'name'=> $form_state['storage']['values'][2]['vp_name'],
								'address'=> $form_state['storage']['values'][2]['vp_address'],
							))->execute();
							
						}
						if(!empty($form_state['storage']['values'][2]['secretary_name'])) {
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'Secretary',
								'name'=> $form_state['storage']['values'][2]['secretary_name'],
								'address'=> $form_state['storage']['values'][2]['secretary_address'],
							))->execute();
						}
						if(!empty($form_state['storage']['values'][2]['treasurer_name'])) {
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'Treasurer',
								'name'=> $form_state['storage']['values'][2]['treasurer_name'],
								'address'=> $form_state['storage']['values'][2]['treasurer_address'],
							))->execute();
						}
						if(!empty($form_state['storage']['values'][2]['director1_name'])) {	
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'Director',
								'name'=> $form_state['storage']['values'][2]['director1_name'],
								'address'=> $form_state['storage']['values'][2]['director1_address'],
							))->execute();
						}
						
						if(!empty($form_state['storage']['values'][2]['director2_name'])) {						
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'Director',
								'name'=> $form_state['storage']['values'][2]['director2_name'],
								'address'=> $form_state['storage']['values'][2]['director2_address'],
							))->execute();
							
						}
						if(!empty($form_state['storage']['values'][2]['director3_name'])) {
							$result = db_insert('wra_renewals_party')
							->fields(array(
								'renewal_id'=> $form_state['storage']['renewal']->renewal_id,
								'title'=> 'Director',
								'name'=> $form_state['storage']['values'][2]['director3_name'],
								'address'=> $form_state['storage']['values'][2]['director3_address'],
							))->execute();
						}
					}
						
				}
				
				if ($order === FALSE) {
				drupal_set_message(t('Order @order_id does not exist.', array('@order_id' => $orderid)));
		//		drupal_goto('admin/store/orders');
				}
				
				$recipient = check_plain($user->mail);
				
				$params = array('order' => $order);
				$sent = '';
				//$sent = drupal_mail('uc_order', 'invoice', $recipient, uc_store_mail_recipient_language($recipient), $params, uc_store_email_from());
				
				if (!$sent) {
					drupal_set_message(t('E-mail failed.'));
				} else {
					$message = t('Invoice e-mailed to @email.', array('@email' => $recipient));
					drupal_set_message($message);
					uc_order_log_changes($orderid, array($message));
				}
				drupal_redirect_form($form, 'payment_done/'.$orderid);
			} else {
//				form_set_error('cc_num',t('Credit card declined:' . $result["r_error"]),TRUE);
				drupal_set_message('Credit card declined:' . $result["r_error"], 'error');
				$form_state['storage']['step'] = 4;
			}
		}
	};
		
	$form_state['rebuild'] = TRUE;

}

/*
 * Calculate renewal fees 
 */
function wysos_renewal_form_calc()
{
	$args = array();
	$form_state = array('storage' =>  NULL, 'submitted' => FALSE);
	$form_build_id = isset($_POST['form_build_id'])?$_POST['form_build_id']:'' ;
	$form = form_get_cache($form_build_id, $form_state);
	$args[] = $form['#parameters'];
	$form_id = array_shift($args);
	$form['#post'] = $_POST;
	$form['#redirect'] = FALSE;
	$form['#programmed'] = FALSE;
	$form_state['post'] = $_POST;
	// Skip Validation
	$form_state['project_issue_ahah'] = TRUE;

	//$form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
	//$form = drupal_rebuild_form($form_id, $form_state,$args, $old_form = NULL);
	
	$total = isset($form_state['post']['ra_price'])?$form_state['post']['ra_price']:'';
	if(isset($form_state['post']['ar']))
		$total += $form_state['post']['ar_price'];
	if(isset($form_state['post']['nominee']))
		$total += $form_state['post']['nominee_price'];
	if(isset($form_state['post']['vo']))
		$total += $form_state['post']['vo_price'];
	if(isset($form_state['post']['penalty']))
		$total += $form_state['post']['penalty_price'];
	if(isset($form_state['post']['reinstate']))
		$total += $form_state['values']['reinstate_price'];
		
	setlocale(LC_MONETARY, 'en_US');
	$form_state['post']['total'] = $total;
	$form_state['values']['total'] = $total;
//	drupal_set_message('total:'.$total);
	$form['total-wrapper']['total'] = array(
		'#type' => 'hidden',
		'#value' => $total,
		'#prefix' => '<tr><td></td><td colspan=2>Total</td><td><b><div id="total-wrapper">',
		'#suffix' => $total.'</div></td></tr></table><p>&nbsp;</p>'
	);
//	drupal_set_message('3:'.print_r($form_state,1));
	
	// re-enable validation
	$form_state['project_issue_ahah'] = FALSE;
	$output =  "<INPUT TYPE='hidden' name='total' value=".$total.">".$total; 
		
	return drupal_json_output(array('status' =>  TRUE, 'data' => $output));
	//exit();
}		
			
/*
 * wysos_pay()
 *
 */
function wysos_pay() {
	global $user;
	$output = t('Pay a WyomingRegisteredAgent.com, Inc. Invoice');
	
	if($user->uid == 0) {
		$output .= drupal_get_form('wysos_pay_invoicenoform');
	} else {
		// Logged In
		$query = 'SELECT invoice_no, filing_name, DATE_FORMAT(due_date,\'%c-%e-%Y\') AS due_date, ttotal_due FROM wra_renewals WHERE payment_date Is Null AND uid=' . $user->uid;
		$query .= ' UNION ';
		$query .= 'SELECT CONV(uc_orders.order_id,10,10) AS invoice_no, billing_company AS company, DATE_FORMAT(Now(),\'%c-%e-%Y\') AS due_date, ';
		$query .= 'CONCAT(\'$\',FORMAT(If(Sum(uc_payment_receipts.amount) IS NULL, order_total, uc_orders.order_total- Sum(uc_payment_receipts.amount)),2)) AS amt_due ';
		$query .= 'FROM uc_orders LEFT JOIN uc_payment_receipts ON uc_orders.order_id = uc_payment_receipts.order_id ';
		$query .= 'GROUP BY uc_orders.uid, uc_orders.order_id, uc_orders.billing_company, uc_orders.order_total, Now() ';
		$query .= 'HAVING uc_orders.uid=' . $user->uid . ' AND (uc_orders.order_total > (SELECT SUM(amount) FROM uc_payment_receipts ';
		$query .= 'WHERE uc_payment_receipts.order_id = uc_orders.order_id) OR Sum(uc_payment_receipts.amount) IS NULL) ';
		$query .= 'ORDER BY filing_name';
		$result = db_query($query)->fetchAll();
		
		if (!$result) {
			die('Invalid query: ' . $query);
		}  
//		$rows = db_fetch_array($result);
		
		// Table tag attributes
		$attributes = array(
		'border'      => 0,
		'cellspacing' => 0,
		'cellpadding' => 3,
		'align'       => 'center'
		);
		
		// Header
		$header = array( 'Invoice No.', 'Company Name', 'Due Date', 'Amount Due' );
		
		foreach ($result as $rows) {
			if( preg_match('/^[Rr][0-9]{6}$/', $rows->invoice_no )) {
				// Renewal Row
				$row[]  = array(
					'style' => 'background-color: LightGrey;',
					'data'  => 	array(array(
							'data' => l($rows->invoice_no, 'renew/' . $rows->invoice_no)),
						array('data' => $rows->filing_name),
						array('data' => $rows->due_date),
						array('data' => $rows->ttotal_due)));
			} else {
				// Invoice Row
				$row[]  = array(
					'style' => 'background-color: LightGrey;',
					'data'  => 	array(array(
						'data' => l($rows->invoice_no, 'pay/' . $rows->invoice_no)),
						array('data' => $rows->filing_name),
						array('data' => $rows->due_date),
						array('data' => $rows->ttotal_due)));
			}
		}	
		// Render Table
		$output = theme('table', array('header'=>$header,'rows'=>$row));
	  return $output;	
	}
	return $output;
}	

function wysos_pay_invoice_form($form, $form_state, $order_id) {
	$order = uc_order_load($order_id);
	if(!$order)
		return drupal_access_denied();
	
	if(!include_once('/sites/all/modules/ubercart/uc_order/uc_order.admin.inc'))
		drupal_set_message('include error');
	$output = uc_order_view_invoice($order, 'view');

	$form['invoice'] = array(
		'#title' => t("Invoice"),
		'#prefix' => '<div id="invoice">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => FALSE
	);
	$output = str_replace("You can pay your invoice online by going to: <a href='https://my.wyomingregisteredagent.com/pay/".$order_id."'>https://my.wyomingregisteredagent.com/pay/".$order_id."</b></a>", '', $output);
	$form['invoice']['list'] = array('#type' => 'markup', '#value' => $output);
	
	return ($form);

}

function wysos_pay_vo_invoice_form($form, &$form_state, $suite_id, $order_id) {


	if(db_query("SELECT COUNT(*) FROM uc_orders o INNER JOIN wra_suites s ON o.uid=s.uid WHERE o.order_id=:order_id AND s.suite_id=:suite_id", array(':order_id'=>$order_id, ':suite_id'=>$suite_id))->fetchField() < 1) 
		return drupal_access_denied(); 
	
	$form_state['storage']['order_id'] = $order_id;
	$form_state['storage']['suite_id'] = $suite_id;

	$order = uc_order_load($order_id);
	if(!$order)
		return drupal_access_denied();

	$form_state['storage']['uid'] = $order->uid;
	$form_state['storage']['order_total'] = $order->order_total;
	
	if(!include_once('/sites/all/modules/ubercart/uc_order/uc_order.admin.inc'))
		drupal_set_message('include error');
	$order_output = uc_order_view_invoice($order, 'view');
		
	$suite = db_query("SELECT street_address, suite_no, price FROM wra_suites WHERE suite_id=:suite_id", array(':suite_id'=>$suite_id));
	//$suite = db_fetch_array($result);
	$form_state['storage']['suite'] = $suite;
	
	$result = db_query("SELECT company_name FROM wra_companies WHERE suite_id=:suite_id ORDER BY company_name ASC", array(':suite_id'=>$suite_id));
	foreach($result as $company_name) {
		$company[] = $company_name;
	};
	$form_state['storage']['company'] = $company;
	$row[]  = array(
		'data'  => 	array(
			array('data' => '<b>Company(ies):</b>'),
			array('data' => implode('<br />', $company))));
	$row[]  = array(
		'data'  => 	array(
			array('data' => '<b>Address:</b>'),
			array('data' => $suite->street_address.' #'.$suite->suite_no.'<br />Cheyenne, WY 82001')));
	$output = theme('table', array('header'=>$header, 'rows'=>$row));

	$form['vo_table'] = array(
		'#type' => 'markup',
		'#value' => $output
	);

	$form['payment'] = array(
		'#title' => t("Payment"),
		'#prefix' => '<div id="payment">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => FALSE
	);
	$form['payment']['order_total'] = array(
		'#type' => 'item',
		'#title' => 'Total Due',
		'#value' => uc_currency_format($order->order_total)
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
		  '#default_value' => $_POST['cc_type'],
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
	
	$form['payment']['cc_exp_month'] = uc_select_month(t('Expiration Month'), $_POST['cc_exp_month']);
	$form['payment']['cc_exp_year'] = uc_select_year(t('Expiration Year'), $_POST['cc_exp_year']);
	
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

	$form['payment']['keep_cc'] = array(
		'#type' => 'checkbox',
		'#title' => 'Use this card for future office charges.',
		'#default_value' => FALSE,
	);
	$form['payment']['email_copy'] = array(
		'#type' => 'checkbox',
		'#title' => 'Email me a copy of the paid invoice.',
		'#default_value' => FALSE,
	);
	$form['payment']['submit-process'] = array(
		'#type' => 'submit',
		'#value' => 'Process Payment'
		);

	$form['invoice'] = array(
		'#title' => t("Invoice"),
		'#prefix' => '<div id="invoice">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => FALSE
	);
	$order_output = str_replace("You can pay your invoice online by going to: <a href='https://my.wyomingregisteredagent.com/pay/".$order_id."'>https://my.wyomingregisteredagent.com/pay/".$order_id."</b></a>", '', $order_output);
	$form['invoice']['list'] = array('#type' => 'markup', '#value' => $order_output);
	$form['cancel_vo'] = array(
		'#title' => t("Cancel Service"),
		'#prefix' => '<div id="cancel_vo">',
		'#suffix' => '</div>',
		'#type' => 'fieldset',
		'#collapsible' => TRUE, 
		'#collapsed' => FALSE
	);
	$form['cancel_vo']['cancel_check'] = array(
		'#type' => 'checkbox',
		'#title' => 'Cancel the office service effective immediately.',
		'#default_value' => FALSE,
	);
	$form['cancel_vo']['submit-cancel'] = array(
		'#type' => 'submit',
		'#value' => 'Cancel Service'
		);
	
	return ($form);
	
}	
function wysos_payment_done($orderid) {
	global $user;
	$output = array();
	if ($user->uid == 0) {
		$output = 'Log in to view your completed order #'.$orderid;
		return $output;
	} else {
	
		$output = 'View your completed '.l('order #'.$orderid,'user/'.$user->uid.'/order/'.$orderid).'.';
		return $output;
	}
}	

function wysos_pay_order($orderno) {
	$color = 'color: SeaGreen;';
	
	$rows = db_query("SELECT order_status, order_total, delivery_company, (order_total-IFNULL((SELECT SUM(amount) FROM uc_payment_receipts WHERE order_id=$orderno), 0)) AS due FROM uc_orders WHERE order_id=$orderno")->fetchObject();
	
	if (!$rows) {
		die('Invalid query: ');
	}  
	//$rows = db_fetch_array($result);

	// Table tag attributes
	$attributes = array(
	'border'      => 0,
	'cellspacing' => 0,
	'cellpadding' => 3,
	'align'       => 'center'
	);
	
	// Row 1
	$row[]  = array(
	'style' => 'background-color: Black;',
	'data'  => array(array(
	'data' => 'Invoice Number: ' . $orderno,
	'colspan' => '2',
	'style' => 'color: White;font-weight: bold;'))); 

	// Row 2
	$row[]  = array(
		'style' => 'background-color: LightGrey;',
		'data'  => 	array(array(
				'data' => 'Company:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => isset($rows->delivery_company)?$rows->delivery_company:'',
				'style' => $color)));
		
	// Row 3
	$due = isset($rows->due)?$rows->due:'';
	$row[]  = array(
		'style' => 'background-color: LightGrey;',
		'data'  => 	array(array(
				'data' => 'Amount Due:',
				'style' => 'font-weight: bold;text-align:right;'),
			array('data' => '$' . number_format($due,2),
				'style' => $color)));

	// Set Session variables
	$_SESSION['total_due'] = $rows->due;
	$_SESSION['invoiceno'] = $orderno;
	//$output = array();
	// Render Table
	$output = theme('table',array( 'rows'=>$row, 'attributes'=>$attributes));

	if($rows->due > 0)
	{
		$output .= '<br><img src="/sites/all/modules/ubercart/payment/uc_credit/images/visa.gif" style="position: relative; top: 5px;"> <img src="/sites/all/modules/ubercart/payment/uc_credit/images/mastercard.gif" style="position: relative; top: 5px;"> <img src="/sites/all/modules/ubercart/payment/uc_credit/images/discover.gif" style="position: relative; top: 5px;"> <img src="/sites/all/modules/ubercart/payment/uc_credit/images/amex.gif" style="position: relative; top: 5px;"><br>';
		$output .= drupal_get_form('wysos_cc_form', $orderno, $rows['order_total']);
	} else {
		$output .= "Paid in full. Thank you";
	}
//	$output .= drupal_get_form('uc_cim_edit_card_form', 3);
//	$output .= drupal_get_form('wysos_renew_entity_invoicenoform2');

	return $output;
}	
/*
 * wysos_pay_order()
 *
 */
function wysos_pay_order2($orderno){
	$color = 'color: SeaGreen';
	
	$rows = db_query("SELECT order_status, order_total, delivery_company FROM uc_orders WHERE order_id=:orderno",array(':orderno'=>$orderno))->fetchObject();
	if(!$rows){
		die('Invalid query');
	}
	// Table tag attributes
	$attributes = array(
		'border' => 0,
		'cellspacing' => 0,
		'cellpadding' => 3,
		'align' => 'center'
	);
	// Row 1
	$row[] = array(
		'style' => 'background-color: Black',
		'data' => array(array(
			'data' => 'Invoice Number:',
			'colspan' => '2',
			'style' => 'color:White; font-weight:bold;')));
	// Row 2
	$row[] = array(
		'style' => 'background-color:LightGrey;',
		'data' => array(array(
			'data' => 'Company:',
			'style' =>'font-weight:bold; text-align:right;'),
			array('data' => $rows->delivery_company,'style' => $color)));
	// Row 3
	$row[] = array(
		'style' => 'background-color:LightGrey;',
		'data' => array(array(
			'data' => 'Amount Due:',
			'style' => 'font-weight:bold;text-align:right'),
			array('data'=> '$'.number_format($rows->order_total,2),  
			'style' => $color)));
	
	//Set Session variables
	$_SESSION['total_due'] = $rows->order_total;
	$_SESSION['invoiceno'] = $orderno;
	
	//Render table
	$output = theme('table',array('rows'=>$row, 'attributes'=>$attributes));
	$output .= '<br><img src="/sites/all/modules/ubercart/payment/uc_credit/images/visa.gif" style="position:relative; top:5px;"> <img src="/sites/all/modules/ubercart/payment/uc_credit/images/mastercard.gif" style="position:relative; top:5px;"> <img src="/sites/all/modules/ubercart/payment/uc_credit/images/discover.gif" style="position:relative;top:5px;"><img src="/sites/all/modules/ubercart/payment/images/amex.gif" style="position:relative;top:5px;"><br>';
	
	$output .= drupal_get_form('wysos_renew_entity_invoicenoform');
	return $output;
}

function _wysos_cc_validate(&$form_state) {
		// Ensure number doesn't overrun.
	$cc_num = substr($form_state['values']['cc_number'], 0, 30);
	
	// Remove non-numeric characters.
	$cc_num = preg_replace('/[^0-9]/', '', $cc_num);
	$form_state['values']['cc_number'] = $cc_num;
	
	// Set up variables.
	
	$CCNumberLeft  = substr($cc_num, 0, 4);
	$CCNumberRight = substr($cc_num, -4);
	$NumberLength = strlen($cc_num);
	
	// Determine the card type and appropriate length.
	if (($CCNumberLeft >= 3000) && ($CCNumberLeft <= 3059)) {
		$ShouldLength = 14;
	};
	if (($CCNumberLeft >= 3600) && ($CCNumberLeft <= 3699)) {
		$ShouldLength = 14;
	};
	if (($CCNumberLeft >= 3800) && ($tCCNumberLeft <= 3889)) {
		$ShouldLength = 14;
	};
	if (($CCNumberLeft >= 3400) && ($CCNumberLeft <= 3499)) {
		$ShouldLength = 15;
	};
	if (($CCNumberLeft >= 3700) && ($CCNumberLeft <= 3799)) {
		$ShouldLength = 15;
	};
	if (($CCNumberLeft >= 3088) && ($CCNumberLeft <= 3094)) {
		$ShouldLength = 16;
	} 
	if (($CCNumberLeft >= 3096) && ($CCNumberLeft <= 3102)) {
		$ShouldLength = 16;
	};
	if (($CCNumberLeft >= 3112) && ($CCNumberLeft <= 3120)) {
		$ShouldLength = 16;
	};
	if (($CCNumberLeft >= 3158) && ($CCNumberLeft <= 3159)) {
		$ShouldLength = 16;
	};
	if (($CCNumberLeft >= 3337) && ($CCNumberLeft <= 3349)) {
		$ShouldLength = 16;
	};
	if (($CCNumberLeft >= 3528) && ($CCNumberLeft <= 3589)) {
		$ShouldLength = 16;
	};
	if (($CCNumberLeft >= 3890) && ($CCNumberLeft <= 3899)) {
		$ShouldLength = 14;
	};
	if (($CCNumberLeft >= 4000) && ($CCNumberLeft <= 4999)) {
		if ($NumberLength > 14) {
			$ShouldLength = 16;
		} 
		if ($NumberLength < 14) {
			$ShouldLength = 13;
		}
	};
	if (($CCNumberLeft >= 5100) && ($CCNumberLeft <= 5599)) {
		$ShouldLength = 16;
	};
	if ($CCNumberLeft == 6011) {
		$ShouldLength = 16;
	};
	
	// Check length.
	if ($NumberLength <> $ShouldLength) {
		$Missing = $NumberLength - $ShouldLength;
		if ($Missing < 0) {
			form_set_error('cc_number','Card number too short.');
		} else {
			form_set_error('cc_number','Card number too long.');
		}
	}
	
	// Mod10 checksum process...
	$Checksum = 0;
	/*
	 * Add even digits in even length strings
	 * or odd digits in odd length strings.
	 */
	for ($Location = 1 - ($NumberLength % 2); $Location < $NumberLength; $Location += 2) {
		$Checksum += substr($cc_num, $Location, 1);
	}

	/*
	 * Analyze odd digits in even length strings
	 * or even digits in odd length strings.
	 */
	for ($Location = ($NumberLength % 2); $Location < $NumberLength; $Location += 2) {
		$Digit = substr($cc_num, $Location, 1) * 2;
		if ($Digit < 10) {
			$Checksum += $Digit;
		} else {
			$Checksum += $Digit - 9;
		}
	}

	// Checksums not divisible by 10 are bad.
	if ($Checksum % 10 != 0) {
		form_set_error('cc_number','Invalid card number (LUHN check).');
	}
	
	// Expiration date process...
	if ($form_state['values']['cc_exp_year'] < date('Y')) {
		form_set_error('cc_number','Card year expired.');
	} elseif ($form_state['values']['cc_exp_year'] == date('Y')) {
		if ($form_state['values']['cc_exp_month'] < date('m')) {
			form_set_error('cc_number','Card expired.');
		}
	}
}























	
