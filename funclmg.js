function $(id) {
	return document.getElementById(id);
}

function getRandomColor() {
	var color = '#' + ('00000' + (Math.random() * 0x1000000 << 0).toString(16)).substr(-6);
	return color;
}

function FunWork(f, x) {
	switch(f) {
		case "":
			{
				return x;
				break;
			}
		case "sin":
			{
				return Math.sin(x);
				break;
			}
		case "cos":
			{
				return Math.cos(x);
				break;
			}
		case "tan":
			{
				return Math.tan(x);
				break;
			}
		case "abs":
			{
				return Math.abs(x);
				break;
			}
		case "sqrt":
			{
				return Math.sqrt(x);
				break;
			}
		case "ln":
			{
				return Math.log(x);
				break;
			}
		case "log":
			{
				return Math.log(x) / Math.LN2;
				break;
			} //2为底
		case "lg":
			{
				return Math.log(x) / Math.LN10;
				break;
			} //10为底
		case "floor":
			{
				return Math.floor(x);
				break;
			}
		case "ceil":
			{
				return Math.ceil(x);
				break;
			}
		case "int":
			{
				return parseInt(x);
				break;
			}
		default:
			{
				return NaN;
				break;
			}
	}
}

function ChangeToPointY(y) {
	return FUN_IMG_HEIGHT - 1 - parseInt((y - yLeftValue) / (yRightValue - yLeftValue) * FUN_IMG_HEIGHT);
}

function isChar(c) {
	return(c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

function isDigit(c) {
	return c >= '0' && c <= '9';
}

function priority(c) {
	switch(c) {
		case '(':
			return 0;
			break;
		case '+':
			return 1;
			break;
		case '-':
			return 1;
			break;
		case '*':
			return 2;
			break;
		case '/':
			return 2;
			break;
		case '^':
			return 3;
			break;
		default:
			return -1;
			break;
	}
}
// 是运算符
function isOpt(c) {
	return priority(c) != -1;
}

function singleCalc(c, a, b) {
	if(c == '+') {
		return a + b;
	} else
	if(c == '-') {
		return a - b;
	} else
	if(c == '*') {
		return a * b;
	} else
	if(c == '/') {
		return a / b;
	} else
	if(c == '^') {
		return Math.pow(a, b);
	} else {
		return NaN;
	}
}

function getTable() {
	tmp = (xRightValue - xLeftValue + EPS) / 20;

	tableX = 1;
	countX = 0;
	countY = 0;
	while(tableX < tmp) {
		tableX *= 10;
	}
	while(tableX / 10 > tmp) {
		tableX /= 10;
		countX++;
	}
	if(tableX >= 1) {
		countX = 0;
	}
	if(tableX / 5 > tmp) {
		tableX /= 5;
		countX++;
	} else if(tableX / 2 > tmp) {
		tableX /= 2;
		countX++;
	}
	var i = parseInt(xLeftValue / tableX) + (xLeftValue > 0)
	for(; i * tableX < xRightValue; i++) {
		if(i == 0) {
			// y轴
			CONTEXT_2D.fillStyle = "black";
		} else {
			// 普通竖线
			CONTEXT_2D.fillStyle = "#CDB7B5";
		}
		tmp = (i * tableX - xLeftValue) / (xRightValue - xLeftValue) * FUN_IMG_WIDTH;
		var _width = 1;
		var _height = FUN_IMG_HEIGHT;
		CONTEXT_2D.fillRect(tmp, 0, _width, _height);
		// 竖线上的数字
		CONTEXT_2D.fillStyle = "red";
		CONTEXT_2D.font = FONT_STYLE;
		var _text = (i * tableX).toFixed(countX);
		var _x = tmp + 2;
		var _y = 10;
		CONTEXT_2D.fillText(_text, _x, _y);
	}
	tmp = (yRightValue - yLeftValue + EPS) / 20;
	tableY = 1;

	while(tableY < tmp) {
		tableY *= 10;
	}
	while(tableY / 10 > tmp) {
		tableY /= 10, countY++;
	}
	if(tableY / 5 > tmp) {
		tableY /= 5, countY++;
	} else if(tableY / 2 > tmp) {
		tableY /= 2, countY++;
	}
	if(tableY >= 1) {
		countY = 0;
	}
	var i = parseInt(yLeftValue / tableY) + (yLeftValue > 0);
	for(; i * tableY < yRightValue; i++) {
		// 横线
		if(i == 0) {
			// x轴
			CONTEXT_2D.fillStyle = "black";
		} else {
			// 普通横线
			CONTEXT_2D.fillStyle = "#CDB7B5";
		}
		tmp = (i * tableY - yLeftValue) / (yRightValue - yLeftValue) * FUN_IMG_HEIGHT;
		CONTEXT_2D.fillRect(0, FUN_IMG_HEIGHT - 1 - tmp, FUN_IMG_WIDTH, 1);
		// 横线上的数字
		CONTEXT_2D.fillStyle = "blue";
		CONTEXT_2D.font = FONT_STYLE;
		CONTEXT_2D.fillText((i * tableY).toFixed(countY), 0, FUN_IMG_HEIGHT - 1 - tmp);
	}
}

function drawArc(x, y) {
	CONTEXT_2D.beginPath();
	// arc(弧形),画圆
	CONTEXT_2D.arc(x, y, 1, 0, Math.PI * 2);
	CONTEXT_2D.closePath();
	CONTEXT_2D.fill();
}

function drawLine(lx, ly, px, py) {
	CONTEXT_2D.beginPath();
	CONTEXT_2D.moveTo(lx, ly);
	CONTEXT_2D.lineTo(px, py);
	CONTEXT_2D.closePath();
	CONTEXT_2D.stroke(); // 绘制
}

function reDraw() {
	CONTEXT_2D.clearRect(0, 0, FUN_IMG_WIDTH, FUN_IMG_HEIGHT);
	getTable();
	getFunction();
}

function change() {
	xLeftValue = parseFloat($("xLeftValue").value);
	xRightValue = parseFloat($("xRightValue").value);
	yLeftValue = parseFloat($("yLeftValue").value);
	yRightValue = parseFloat($("yRightValue").value);
	reDraw();
}

function update() {
	$("xLeftValue").value = xLeftValue;
	$("xRightValue").value = xRightValue;
	$("yLeftValue").value = yLeftValue;
	$("yRightValue").value = yRightValue;
}

function scale(x, y, times) {
	if(x < 0 || x >= FUN_IMG_WIDTH || y < 0 || y >= FUN_IMG_HEIGHT) return;

	if(times < 1 && (xRightValue - xLeftValue < MIN || yRightValue - yLeftValue < MIN)) {
		return;
	}
	if(times > 1 && (xRightValue - xLeftValue > MAX || yRightValue - yLeftValue > MAX)) {
		return;
	}

	x = xLeftValue + (xRightValue - xLeftValue) / FUN_IMG_WIDTH * x;
	y = yLeftValue + (yRightValue - yLeftValue) / FUN_IMG_HEIGHT * y;
	xLeftValue = x - (x - xLeftValue) * times;
	xRightValue = x + (xRightValue - x) * times;
	yLeftValue = y - (y - yLeftValue) * times;
	yRightValue = y + (yRightValue - y) * times;
}

function Calc(fun, X, Value) {
	var number = new Array(),
		opt = new Array(),
		F = new Array(),
		now = 0,
		f = "",
		tmp, a, b, sign = 1,
		base = 0,
		j;
	for(var i = 0; i < fun.length; i++) {
		for(j = 0; j < X.length; j++)
			if(X[j] == fun[i]) {
				if(i == 0 || isOpt(fun[i - 1])) now = Value[j];
				else now *= Value[j];
				break;
			}
		if(j == X.length)
			if(fun[i] == '(') F.push(f), f = "", opt.push('(');
			else
		if(fun[i] == ')') {
			number.push(now * sign);
			now = 0;
			sign = 1;
			base = 0;
			while((tmp = opt.pop()) != '(') {
				b = number.pop();
				a = number.pop();
				tmp = singleCalc(tmp, a, b);
				number.push(tmp);
			}
			now = FunWork(F.pop(), number.pop());
		} else
		if(fun[i] == '.') base = 1;
		else
		if(fun[i] == '+' && (i == 0 || priority(fun[i - 1]) != -1));
		else
		if(fun[i] == '-' && (i == 0 || priority(fun[i - 1]) != -1)) sign = -1;
		else
		if(fun[i] == 'e')
			if(i == 0 || isOpt(fun[i - 1])) now = Math.E;
			else now *= Math.E;
		else
		if(fun[i] == 'p' && fun[i + 1] == 'i') {
			if(i == 0 || isOpt(fun[i - 1])) now = Math.PI;
			else now *= Math.PI;
			i += 1;
		} else
		if(isDigit(fun[i]))
			if(base == 0) now = now * 10 + (fun[i] - '0');
			else base /= 10, now += base * (fun[i] - '0');
		else
		if(isChar(fun[i])) f += fun[i];
		else if(isOpt(fun[i])) {
			number.push(now * sign);
			now = 0;
			sign = 1;
			base = 0;
			var s = priority(fun[i]);
			if(s == -1) return 0;
			while(s <= priority(opt[opt.length - 1])) {
				b = number.pop();
				a = number.pop();
				tmp = singleCalc(opt.pop(), a, b);
				number.push(tmp);
			}
			opt.push(fun[i]);
		}
	}
	number.push(now * sign);
	while(opt.length > 0) {
		b = number.pop();
		a = number.pop();
		tmp = singleCalc(opt.pop(), a, b);
		number.push(tmp);
	}
	return number.pop();
}

function getFunction() {
	// group：函数（可能是复数）
	var group = document.getElementsByName("Fun");
	var x, y;
	var lax, lay;
	var px, py
	var color, outSide, type
	var ValueL, ValueR, ValueS, isDrawLine, tmp, TMP;

	for(var k = 1; k < group.length; k++) {

		var _funcItem = group[k].parentNode;

		outSide = 1;
		//type = _funcItem.children[0].value;
		// 颜色
		color = _funcItem.children[0].value;
		// 函数表达式
		funcExpression = group[k].value;
		// 是否画线（默认画点）
		isDrawLine = _funcItem.children[3].checked;

		CONTEXT_2D.fillStyle = CONTEXT_2D.strokeStyle = color;

		for(var i = 0; i < FUN_IMG_WIDTH; i++) {
			x = xLeftValue + (xRightValue - xLeftValue) / FUN_IMG_WIDTH * i;
			y = Calc(funcExpression, ['x'], [x]);
			if(isNaN(y)) {
				continue;
			}
			px = i;
			py = ChangeToPointY(y);
			if(y >= yLeftValue && y < yRightValue) {
				// 画圆
				drawArc(px, py);
				if(isDrawLine) {
					drawLine(lax, lay, px, py);
				}
				outSide = 0;
			} else {
				if(isDrawLine) {
					if(!outSide) {
						drawLine(lax, lay, px, py);
					}
				} else {}
				outSide = 1;
			}
			lax = px;
			lay = py;
		}
	}
}

function Add() {
	var newInput = $("mod").cloneNode(true);
	newInput.style.display = "block";
	newInput.children[0].value = getRandomColor();
	$("function").appendChild(newInput);
}

function Delete(node) {
	node.parentNode.removeChild(node);
}
