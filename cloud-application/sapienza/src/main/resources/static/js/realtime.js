//List of CustomChart
var chart_list = [];
var interval_id = null;
var anomaly_table;



$(document).ready(function () {
    $('#gateway_data').submit(function (event) {
        event.preventDefault();


        clearInterval(interval_id);
        generateGraphs();
        generateAnomaliesTable();
        interval_id = setInterval(function() {
            generateGraphs();  // Update graphs
            generateAnomaliesTable();  // Update anomalies table
        }, 5000);
    });


    anomaly_table = $('#anomalies_table').DataTable({
        "paging": true,
        "searching": false, // Disable search box
        "info": false, // Disable info text,
        "pageLength" : 15,
        language: {
            entries: {
                _: 'anomalies',
                1: 'anomaly'
            },
            info: ""
        },
        "ordering" :false
    });

});

function generateGraphs() {


    $.ajax({
        url: '/api/datastream/all',
        type: 'GET',
        data: {
        },
        success: function (data) {
            // Assuming data is an array of objects
            if (data.length !== 0) {
                handleChartCreationAndUpdate("Temperature", data, true);
                disableAnimation();
            }else{
                destroyCharts();
            }

        },
        error: function () {
            alert('Error fetching data');
        }
    });
}

function generateAnomaliesTable(){
    $.ajax({
        url: '/api/anomalies',
        type: 'GET',
        data: {
        },
        success: function (data) {
            // Assuming data is an array of objects
            if (data.length !== 0) {
                fillAnomaliesTable(data) // fill anomalies table with last hour data (limit?)
                disableAnimation();
            }else{

            }

        },
        error: function () {
            alert('Error fetching data');
        }
    });
}

function disableAnimation() {
    chart_list.forEach(c => {
        c.chart.options.animation.duration = 0;
        c.chart.update();
    })
}
// Reset graphs
$('#reset').click(function (event) {
    event.preventDefault();
    chart_list.forEach(c => {
        $('#'.concat(c.canva_name)).attr("class", "0");
        c.chart.destroy();
    })
    chart_list = [];
    if(anomaly_table != null){
        anomaly_table.clear().draw();
        anomaly_table = null;
    }
    clearInterval(interval_id);
})
