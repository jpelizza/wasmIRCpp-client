let chan = '';
let nick = '';
let color = '';
let otherColor = '';

function formIsValid(form) {
  let isValid = true;
  const inputs = $(form + ' input').toArray();
  inputs.forEach(input => {
    if (input.id != 'input-chan-key' && !input.value) {
      isValid = false;
    }
  });
  return isValid;
}

function getRandColor() {
  return '#' + Math.floor(Math.random() * 16777215).toString(16);
}

function appendMsg(user, userColor, msg) {
  $('.messages').append(`<p><span style="color:${userColor};text-shadow:0 0 1rem ${userColor};">${user}: </span></p>`);
  $('.messages p:last-child').append(document.createTextNode(msg));
  $('.messages').scrollTop($('.messages')[0].scrollHeight);
}

function getMsg() {
  const nextMsg = irc.getNextMessage();
  if (nextMsg) {
    const json = JSON.parse(nextMsg);
    if (json.command == 'PRIVMSG' && json.nick && json.nick != nick) {
      if (!otherColor) {
        otherColor = getRandColor();
      }
      appendMsg(json.nick, otherColor, json.trailing);
    }
  }
}

$('#input-color').val(getRandColor());

// Validate "User Register" form
$('#form-register-user input').on('change', (e) => {
  e.preventDefault();
  if (formIsValid('#form-register-user')) {
    $('#btn-register').attr('disabled', false);
    $('#btn-register').removeClass('disabled');
  } else {
    $('#btn-register').attr('disabled', true);
    $('#btn-register').addClass('disabled');
  }
});

// "Register" button
$('#btn-register').on('click', (e) => {
  e.preventDefault();
  const url = $('#input-url').val();
  const port = $('#input-port').val();
  chan = $('#input-chan').val();
  const chanKey = $('#input-chan-key').val();
  const user = $('#input-user').val();
  const host = $('#input-host').val();
  const server = $('#input-server').val();
  const name = $('#input-name').val();
  nick = $('#input-nick').val();
  color = $('#input-color').val();

  module.openWebSocket(url, port);
  setTimeout(() => {
    irc.registerUser(user, host, server, name, nick);
    setTimeout(() => {
      irc.join([chan], [chanKey])
    }, 1500);
    $('.chan').append(chan);
    $('#form-register-user').removeClass('visible');
    $('#form-send-msg').addClass('visible');
    setInterval(() => getMsg(), 250);
  }, 1500);

});

// "Send" button
$('#btn-send').on('click', (e) => {
  e.preventDefault();
  const inputMsg = $('#input-msg');
  if (inputMsg.val()) {
    irc.privmsg(inputMsg.val());
    appendMsg(nick, color, inputMsg.val());
    inputMsg.val('');
  };
});

// "Enter" key
$('#input-msg').on('keyup', (e) => {
  if (e.keyCode == 13) {
    $('#btn-send').trigger('click');
  }
});
