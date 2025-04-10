var sys_data;

//List of CustomChart
var chart_list = [] ;



// Reset graphs
$('#reset').click(function (event) {
    event.preventDefault();
    destroyCharts();

})

$(document).ready(function () {
    $('#log_submit').submit(function (event) {
        event.preventDefault();

        var start_timestamp = $('#start_time').val();
        var end_timestamp = $('#end_time').val();


        $.ajax({
            url: '/api/datastream/timeRange',
            type: 'GET',
            data: {
                start: start_timestamp,
                end: end_timestamp
            },
            success: function (data) {
                // Assuming data is an array of objects
                console.log(data);
                if(data.length !== 0) {
                    handleChartCreationAndUpdate("Temperature", data,false);
                }else{
                    destroyCharts();
                }

            },
            error: function () {
                alert('Error fetching data');
            }
        });
    });


});




