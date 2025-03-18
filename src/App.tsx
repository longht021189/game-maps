import { createSignal, onMount } from 'solid-js';
import './App.css';

var posX!: number;
var posY!: number;
var screenWidth = document.documentElement.clientWidth;
var screenHeight = document.documentElement.clientHeight;

const App = () => {
  let fdiv!: HTMLDivElement;

  function mousemove(ev: any) {
    if (ev == null) {
      ev = window.event;
    } //IE
    if ((ev.clientY - posY) <= 0) { //超过顶部
      fdiv.style.top = "0px";
    } else if ((ev.clientY - posY) > (screenHeight - fdiv.clientHeight)) { //超过底部
      fdiv.style.top = (screenHeight - fdiv.clientHeight) + "px";
    } else {
      fdiv.style.top = (ev.clientY - posY) + "px";
    }
  
    if ((ev.clientX - posX) <= 0) { //超过左边
      fdiv.style.left = "0px";
    } else if ((ev.clientX - posX) > (screenWidth - fdiv.clientWidth)) { //超过右边
      fdiv.style.left = (screenWidth - fdiv.clientWidth) + "px";
    } else {
      fdiv.style.left = (ev.clientX - posX) + "px";
    }
    // console.log( posX +"  "+ fdiv.style.left);
  }

  window.onload = window.onresize = function() { //窗口大小改变事件
    screenWidth = ;
    screenHeight = ;
    if ((parseInt(fdiv.style.top) + fdiv.clientHeight) > screenHeight) { //窗口改变适应超出的部分
      fdiv.style.top = (screenHeight - fdiv.clientHeight) + "px";
    }
    if ((parseInt(fdiv.style.left) + fdiv.clientWidth) > screenWidth) { //窗口改变适应超出的部分
      fdiv.style.left = (screenWidth - fdiv.clientWidth) + "px";
    }
    (document.onmouseup as any)?.(null)
  };

  (window.onload as any)?.();

  const [data, setData] = createSignal<{
    posX: number,
    posY: number,
    screenWidth: number,
    screenHeight: number,
  }>({
    posX: 0,
    posY: 0,
    screenWidth: 0,
    screenHeight: 0,
  });

  onMount(() => {
    setData(value => ({
      ...value,
      screenWidth: document.documentElement.clientWidth,
      screenHeight: document.documentElement.clientHeight,
    }))
  })

  return (
    <div class="content">
      <h1>Rsbuild with Solid</h1>
      <p>Start building amazing things with Rsbuild.</p>

      <div
          ref={fdiv}
          id="drager"
          onmousedown={(e: any) => {
            screenWidth = document.documentElement.clientWidth;
            screenHeight = document.documentElement.clientHeight;
            if (!e) {
              e = window.event;
            } // IE
            posX = e.clientX - parseInt(fdiv.style.left);
            posY = e.clientY - parseInt(fdiv.style.top);
            document.onmousemove = mousemove;
          }}
          onmouseup={() => {
            document.onmousemove = null;
            if ((parseInt(fdiv.style.top) + fdiv.clientHeight / 2) <= (screenHeight / 2)) { //在上半部分
              if ((parseInt(fdiv.style.left) + fdiv.clientWidth / 2) <= (screenWidth / 2)) { //在左半部分
                if ((parseInt(fdiv.style.top) + fdiv.clientHeight / 2) <= (parseInt(fdiv.style.left) + fdiv.clientWidth / 2)) { //靠近上方
                  fdiv.style.top = "0px";
                } else { //靠近左边
                  fdiv.style.left = "0px";
                }
              } else { //在右半部分
                if ((parseInt(fdiv.style.top) + fdiv.clientHeight / 2) <= (screenWidth - (parseInt(fdiv.style.left) + fdiv.clientWidth / 2))) { //靠近上方
                  fdiv.style.top = "0px";
                } else { //靠近右边
                  fdiv.style.left = (screenWidth - fdiv.clientWidth) + "px";
                }
              }
            } else { //下半部分
              if ((parseInt(fdiv.style.left) + fdiv.clientWidth / 2) <= (screenWidth / 2)) { //在左半部分
                if ((screenHeight - (parseInt(fdiv.style.top) + fdiv.clientHeight / 2)) <= (parseInt(fdiv.style.left) + fdiv.clientWidth / 2)) { //靠近下方
                  fdiv.style.top = (screenHeight - fdiv.clientHeight) + "px";
                } else { //靠近左边
                  fdiv.style.left = "0px";
                }
              } else { //在右半部分
                if ((screenHeight - (parseInt(fdiv.style.top) + fdiv.clientHeight / 2)) <= (screenWidth - (parseInt(fdiv.style.left) + fdiv.clientWidth / 2))) { //靠近上方
                  fdiv.style.top = (screenHeight - fdiv.clientHeight) + "px";
                } else { //靠近右边
                  fdiv.style.left = (screenWidth - fdiv.clientWidth) + "px";
                }
              }
            }
          }}
          style={{ top: "100px", left: "100px" }}>
        <div />
      </div>
    </div>
  );
};

export default App;

/* https://codepen.io/harveylee/pen/QErXGQ

fdiv.addEventListener('touchstart', fdiv.onmousedown, false);
fdiv.addEventListener('touchmove', function(event) {
  // 如果这个元素的位置内只有一个手指的话
  if (event.targetTouches.length == 1) {　　　　
    event.preventDefault(); // 阻止浏览器默认事件，重要  
    var touch = event.targetTouches[0];
    if ((touch.pageY) <= 0) { //超过顶部
      fdiv.style.top = "0px";
    } else if (touch.pageY > (screenHeight - parseInt(fdiv.clientHeight))) { //超过底部
      fdiv.style.top = (screenHeight - parseInt(fdiv.clientHeight)) + "px";
    } else {
      fdiv.style.top = (touch.pageY - parseInt(fdiv.clientHeight) / 2) + "px";
    }

    if (touch.pageX <= 0) { //超过左边
      fdiv.style.left = "0px";
    } else if (touch.pageX > (screenWidth - parseInt(fdiv.clientWidth))) { //超过右边
      fdiv.style.left = (screenWidth - parseInt(fdiv.clientWidth)) + "px";
    } else {
      fdiv.style.left = (touch.pageX - parseInt(fdiv.clientWidth) / 2) + "px";
    }
  }
}, false);
fdiv.addEventListener('touchend', document.onmouseup, false);
fdiv.ondblclick = function() { //双击事件可能在手机端浏览器会与网页缩放事件冲突
  alert("发挥你们的想象力吧");
}

*/