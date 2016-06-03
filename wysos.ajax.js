jQuery(document).ready(function(){
	jQuery('#tcountry').change(function(){
		var cid = jQuery('#tcountry').val();
		 jQuery('#edit-zone-load').html('<img src="'+Drupal.settings.basePath+'sites/all/modules/wysos/loading.gif">');
			jQuery.ajax({
				type:"post",
				url:Drupal.settings.basePath+"wysos/zone/list",
				data:{"cid":cid},
				success: function(data){
					var sel = jQuery("#tzone");
					sel.empty();
					if(data =='invalid'){
						jQuery('#edit-zone-load').html('');
						sel.append('<option value="not-applicable">Not Applicable</option>');
						sel.attr('readonly', true);
					}	
					else{
						
						var opts =  jQuery.parseJSON(data);
						// Use jQuery's each to iterate over the opts value
						jQuery.each(opts, function(i, d) {
							// You will need to alter the below to get the right values from your json object.  Guessing that d.id / d.modelName are columns in your carModels data
							//alert(d.name);
							sel.append('<option value="' + d.id + '">' + d.name + '</option>');
							jQuery('#edit-zone-load').html('');
						});
					}		
				}
			},"json");
	});

});