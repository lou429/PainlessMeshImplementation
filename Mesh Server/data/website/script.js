var currentChipId = 1231231;
var currentItem;
var menuState = false;
var elementName = 0;
var selectedId = 1;
var selectedName = "";
var deviceCount = 0;
var messageCounter = 0;
let devices = [{
    id: 92137481,
    name: function () {
        return RetrieveBoardName(this.id);
    },
    state: false
}, {
    id: 818231623,
    name: function () {
        return RetrieveBoardName(this.id);
    },
    state: false
}, {
    id: 7123123,
    name: function () {
        return RetrieveBoardName(this.id);
    },
    state: false
}];

window.onload = function loadDevices() {
    controllerEventHandler();

    let onlineDevices = new Array();

    devices.forEach(device => onlineDevices.push(device.id, device.name()));

    console.log("online device list: \n");
    console.log(onlineDevices);

    var tempList = new Array();

    if (localStorage.hasOwnProperty(currentChipId)) {
        tempList = JSON.parse(localStorage.getItem(currentChipId));
        if (tempList != devices && tempList.length > 0) {
            console.log("Online list: \n" + onlineDevices);
            console.log("Temp list: \n" + tempList);
            onlineDevices.forEach(device => {
                for (i = 0; i != tempList.length; i++)
                    if (device === tempList[i]) {
                        tempList.splice(i, 1);
                        break;
                    }
            });

            if (tempList.length > 0) {
                console.log(tempList);
                try {
                    AddOfflineDevices(tempList);
                    localStorage.setItem(currentChipId, JSON.stringify(onlineDevices + tempList));
                } catch (ex) {
                    console.log(ex);
                }
            }
        }
    } else
        localStorage.setItem(currentChipId, JSON.stringify(onlineDevices));


    deviceCount = devices.length;
    document.getElementById('connected-devices').innerHTML = deviceCount;

    var deviceList = document.getElementById("device-list");

    devices.forEach((device, counter) => {
        var node = document.createElement("li");
        node.setAttribute("id", "device" + counter);
        node.setAttribute("class", "device-info");

        var spanNode = document.createElement("span");
        spanNode.setAttribute("id", "device-name-" + counter);
        spanNode.setAttribute("class", "device-id");
        spanNode.setAttribute("key", device.id);

        if (device.name() != "")
            spanNode.appendChild(document.createTextNode(Capitalize(device.name())));
        else
            spanNode.appendChild(document.createTextNode(device.id));

        var textNode = document.createElement("span");
        textNode.setAttribute("id", "device-state-" + counter);
        textNode.setAttribute("class", "device-state");
        textNode.appendChild(document.createTextNode(device.state ? "On" : "Off"));
        node.appendChild(spanNode);
        node.appendChild(document.createTextNode(" : "));
        node.appendChild(textNode);

        deviceContextMenu(node);

        deviceList.appendChild(node);
    });
}


function deviceContextMenu(item) {
    item.addEventListener("contextmenu", function (e) {
        console.log(item);
        var childNodes = item.childNodes;
        currentItem = item;
        elementName = childNodes[0].getAttribute('id');
        selectedId = childNodes[0].getAttribute("key");
        selectedName = childNodes[0].innerHTML;
        var contextmenu = document.getElementById("context-menu");
        contextmenu.style.display = "block";
        contextmenu.style.left = (event.pageX - 10) + "px";
        contextmenu.style.top = (event.pageY - 10) + "px";
        e.preventDefault();
    }, false);

    document.addEventListener("click", function (event) {
        var contextmenu = document.getElementById("context-menu");
        contextmenu.style.display = "";
        contextmenu.style.left = "";
        contextmenu.style.top = "";
    }, false);

    //Add event listener to allow box to be selected
    item.addEventListener("click", function (e) {
        $(this).toggleClass("Selected");
    });
}

function returnDeviceToNode(device, counter) {
    var node = $("<li></li>");
    $(node).attr("id", "device" + counter);
    $(node).attr("class", "device-info");

    var spanNode = $("<span></span>");
    $(spanNode).attr("id", "device-name-" + counter);
    $(spanNode).addClass("device-id");
    $(spanNode).attr("key", device.id);

    if (device.name != "")
        $(spanNode).text(device.name);
    else if (device.id > 0)
        $(spanNode).text(device.id);
    else
        $(spanNode).text(device)


    var textNode = $("<span></span>").text("Offline").attr("id", "device-state-" + counter);
    $(textNode).toggleClass("state-offline");
    node.append(spanNode);
    node.append(document.createTextNode(" : "));
    node.append(textNode);

    return node;
}

function AddOfflineDevices(list = new Array()) {
    let container = $("<div></div>").attr("id", "device-info");
    container.attr("class", "offline-devices");

    let listContainer = $("<ul></ul>").attr("id", "device-list");
    try {
        list.forEach((device, iterator) => {
            $(listContainer).append(returnDeviceToNode(device, iterator));
        });
    } catch (ex) {
        console.log(ex);
        $(listContainer).append(returnDeviceToNode(list, 0));
    }

    $('#col-1').append($("<br/>"));
    let heading = $("<h3></h3>").text("Offline devices: ");
    $(heading).append($("<span></span>").text(list.length)).attr('id', 'disconnected-devices');
    $("#col-1").append(heading);
    container.append(listContainer);
    $("#col-1").append(container);
}

function RenameSelected() {
    var name = prompt("Please enter name", selectedName != "" ? selectedName : selectedId);
    StoreBoardName(selectedId, name);

    var selectedDevice = $(elementName);
    selectedDevice.text(RetrieveBoardName(selectedId));
}

function RestoreBoardName() {
    localStorage.removeItem(selectedId);
    $(elementName).text(selectedId);
}

function Refresh() {
    location.reload();
}

function Select() {
    $(currentItem).toggleClass("Selected");
}

function SelectAll() {
    $(".device-info").each(() => $(this).toggleClass("Selected"));
}

function ChangeState(id) {
    var selectedDevices = SelectedDevicesId();
}

function SelectedDevicesId() {
    return $(".device-id").map(function () {
        return $(this).attr('key');
    }).get().concat();
}

const Capitalize = (i) => i.charAt(0).toUpperCase() + i.slice(1);

//Set name and save in local storage
function StoreBoardName(id, name) {
    if (typeof Storage !== "undefined")
        localStorage.setItem(id, Capitalize(name));
    else
        alert("Local storage not supported on this device");
}

//Retrieve local name
function RetrieveBoardName(id) {
    if (localStorage.hasOwnProperty(id))
        return localStorage.getItem(id);
    return "";
}

function controllerEventHandler() {
    var currentState = true;
    setInterval(function () {
        if (currentState)
            $("#terminal-ending").hide();
        else
            $("#terminal-ending").show();
        currentState = !currentState;
        enableButton();
    }, 500);
}

function appendToTerminal(message) {
    devices.forEach(device => {
        if (device.name() != "")
            message = message.replace(device.id, device.name());
    });

    var messageEnding = $('<p></p>');
    messageEnding.attr('id', 'terminal-message-' + messageCounter++);
    messageEnding.attr('class', 'terminal-message');
    messageEnding.text("/var/Mesh controller:~$ " + message);

    $(messageEnding).insertBefore($("#terminal-start"));
}

function enableButton() {
    var devices = $(".device-info").map(function () {
        return $(this).attr('key');
    }).get().concat() ?? 0;
    if (devices.length > 0)
        $("#send-button").attr("disabled", "false");
    else
        $("#send-button").attr("disabled", "true");
}