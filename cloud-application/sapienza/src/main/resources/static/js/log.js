//List of CustomChart
var chart_list = [];
var interval_id = null;
var packets_db = [];
var payload_map = new Map();
var packet_table;


$(document).ready(function () {
    $('#packet_data').submit(function (event) {
        event.preventDefault();


        clearInterval(interval_id);
        generateGraphs();
        interval_id = setInterval(generateGraphs, 5000);
    });

    $('#packet_pause').click(function (event) {
        event.preventDefault();
        clearInterval(interval_id);
        console.log("GRAPH PAUSED");
        if(packets_db.length !== 0){
            generateTable(packets_db);
        }

    });

    packet_table = $('#packet_table').DataTable({
        "paging": true,
        "searching": false, // Disable search box
        "info": false, // Disable info text,
        "pageLength" : 15,
        language: {
            entries: {
                _: 'packets',
                1: 'packet'
            },
            info: ""
        }
    });

    $('#reset').click(function (event) {
        event.preventDefault();
        chart_list.forEach(c => {
            $('#'.concat(c.canva_name)).attr("class", "0");
            c.chart.destroy();
        })
        chart_list = [];
        clearInterval(interval_id);
        if(packet_table != null){
            packet_table.clear().draw();
        }
    });








});

function generateGraphs() {

    var gateway_id_list = [];

    $('input[type="checkbox"]:checked').each(function () {
        gateway_id_list.push($(this).val()); // Add the value to the array
    });

    $.ajax({
        url: '/packets/getAllLastFiveMinutesPackets',
        type: 'GET',
        data: {
            id: gateway_id_list,
            start: null,
            end: null
        },
        success: function (data) {
            // Assuming data is an array of objects
            console.log(data);
            if (data.length !== 0) {
                packets_db = data;
                handleChartCreationAndUpdate("PACKETS_TRAFFIC", data, "packets", true);
                disableAnimation();
            } else {
                clearInterval(interval_id);
                destroyCharts();
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


function generateTable(packets_db) {

    var i = 0;
    var newData = [];
    packets_db.forEach(p => {

        var packet = JSON.parse(p.packet);

        var table_payload = "table_payload_" + i;
        let packet_json = JSON.stringify(packet,null,2).toString();
        payload_map.set(table_payload, packet_json);
        var timestamp =  new Date (Date.parse(p.timestamp));

        var timestamp_string = timestamp.getHours() + ":" + timestamp.getMinutes() + ":" + timestamp.getSeconds()+"" ;

        var payload = "<button class=\"rounded-md shadow-lg bg-white ring-1 ring-black ring-opacity-5 m-4 p-2 \" id=" + table_payload + "> Show JSON  </button> </td>\n";

        newData.push([
            timestamp_string,
            packet.mhdr.mtype,
            payload
        ]);

            i++;

    });
    packet_table.clear();
    packet_table.rows.add(newData).draw();

    function handleClick(event, json) {
        $('#json').html(colorize(JSON.parse(json),1));
    }

    payload_map.forEach((v, k) => {
        $("#" + k).on('click', function (event) {
            handleClick(event,v);
        });
    });



}

function colorize(obj, level) {
    if(obj == null || obj == undefined) return `<span class="nullcolor">${obj}</span>`
    if (typeof obj == 'number') return `<span class="numbercolor">${obj}</span>`
    else if (typeof obj == 'string') return `<span class="stringcolor">"${obj}"</span>`

    let isArray = Array.isArray(obj)
    let html = `${level == 1 ? '<span style="color: white;">' : ''}${isArray ? '[' : '{'}<br>`
    if (isArray) {
        obj.forEach(elem => {
            html += `${'&emsp;'.repeat(level)}${colorize(elem,level + 1)},<br>`
        })
    }
    else {
        Object.keys(obj).forEach(key => {
            if (typeof obj[key] == 'object') {
                if (Array.isArray(obj[key])) {
                    html += `${'&emsp;'.repeat(level)}<span class="keycolor">${key}</span>:&nbsp${colorize(obj[key], level + 1)}<br>`
                }
                else html += `${'&emsp;'.repeat(level)}<span class="keycolor">${key}</span>:&nbsp${colorize(obj[key], level + 1)}<br>`
            }
            else if (typeof obj[key] == 'number') html += `${'&emsp;'.repeat(level)}<span class="keycolor">${key}</span>:&nbsp${colorize(obj[key])}<br>`
            else if (typeof obj[key] == 'string') html += `${'&emsp;'.repeat(level)}<span class="keycolor">${key}</span>:&nbsp${colorize(obj[key])}<br>`
            else html += `${'&emsp;'.repeat(level)}<span class="keycolor">${key}</span>:&nbsp<span class="defaultcolor">${obj[key]}</span><br>`
        })
    }
    html += `${'&emsp;'.repeat(level - 1)}${isArray ? ']' : '}'}${level == 1 ? '</span>' : ''}`
    return html
}


