var form;
window.onload = function(){
    form = document.forms['loginform']
    
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
            url: "/api/v1/login",
            type: "POST",
            data: JSON.stringify(data),
            success: function() {
                window.location.href = getParameterByName('next', window.location.href);
            },
            error: function(xhr, resp, text) {
                console.log(xhr, resp, text);
            }
        })
    };
};

function getParameterByName(name, url) {
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, "\\$&");
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}
