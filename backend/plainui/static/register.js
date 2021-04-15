var form;
window.onload = function(){
    form = document.forms['registerform']
    
    form.onsubmit = function (e) {
        // stop the regular form submission
        e.preventDefault();

        // collect the form data while iterating over the inputs
        var data = {};
        for (var i = 0, ii = form.length; i < ii; ++i) {
            var input = form[i];
            if (input.name) {
            data[input.name] = input.value;
            }
        }

        $.ajax({
            url: "/api/v1/register",
            type: "POST",
            data: JSON.stringify(data),
            success: function() {
                window.location.href = "/login";
            },
            error: function(xhr, resp, text) {
                console.log(xhr, resp, text);
            }
        })
    };
};
