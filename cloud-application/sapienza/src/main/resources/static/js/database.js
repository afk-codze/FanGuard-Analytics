var sys_data;

//List of CustomChart
var chart_list = [];



// Reset graphs
$('#reset').click(function (event) {
    event.preventDefault();
    destroyCharts();

})

$(document).ready(function () {
    $('#gateway_data').submit(function (event) {
        event.preventDefault();

        var gateway_id_list = [];
        var start_timestamp = $('#start_time').val();
        var end_timestamp = $('#end_time').val();


        $('input[type="checkbox"]:checked').each(function () {
            gateway_id_list.push($(this).val()); // Add the value to the array
        });

        $.ajax({
            url: '/resources/getAllById&Timestamp',
            type: 'GET',
            data: {
                id: gateway_id_list,
                start: start_timestamp,
                end: end_timestamp
            },
            success: function (data) {
                // Assuming data is an array of objects
                console.log(data);
                if(data.length !== 0) {
                    handleChartCreationAndUpdate("CPU_Usage", data, "cpu_usage");
                    handleChartCreationAndUpdate("RAM_Usage", data, "ram_usage");
                    handleChartCreationAndUpdate("NET_Usage", data, "net");
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




