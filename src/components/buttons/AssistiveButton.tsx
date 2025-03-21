import { type Accessor, createMemo, createSignal, onCleanup, onMount, type Setter } from 'solid-js'
import { settingsPageVisibilitySignal } from '../pages/settings/SettingsPage';

const buttonSize = 50;

type Position = {
  x: number
  y: number
}

type AssistiveButtonThis = {
  position: Accessor<Position>
  setPosition: Setter<Position>
  isDragging: Accessor<boolean>
  setIsDragging: Setter<boolean>
}

function startDrag(this: AssistiveButtonThis, e: MouseEvent | TouchEvent) {
  e.preventDefault();
  this.setIsDragging(true);
}

function onDrag(this: AssistiveButtonThis, e: MouseEvent | TouchEvent) {
  if (this.isDragging()) {
    const clientX = "touches" in e ? e.touches[0].clientX : e.clientX;
    const clientY = "touches" in e ? e.touches[0].clientY : e.clientY;
    this.setPosition({ x: clientX - buttonSize / 2, y: clientY - buttonSize / 2 });
  }
}

function stopDrag(this: AssistiveButtonThis) {
if (this.isDragging()) {
  this.setIsDragging(false);

      // Get current position and viewport dimensions
      const currentPos = this.position();
      const viewportWidth = window.innerWidth;
      const viewportHeight = window.innerHeight;

      // Calculate distances to each edge
      const leftDist = currentPos.x;
      const rightDist = viewportWidth - (currentPos.x + buttonSize);
      const topDist = currentPos.y;
      const bottomDist = viewportHeight - (currentPos.y + buttonSize);

      // Find the minimum distance to snap to the nearest edge
      const minDist = Math.min(leftDist, rightDist, topDist, bottomDist);
      let newX = currentPos.x;
      let newY = currentPos.y;

      if (minDist === leftDist) {
        newX = 0; // Snap to left
      } else if (minDist === rightDist) {
        newX = viewportWidth - buttonSize; // Snap to right
      }

      if (minDist === topDist) {
        newY = 0; // Snap to top
      } else if (minDist === bottomDist) {
        newY = viewportHeight - buttonSize; // Snap to bottom
      }

      // Update position with snapping
      this.setPosition({ x: newX, y: newY });
    }
}

const AssistiveButton = () => {
  const [position, setPosition] = createSignal<Position>({ x: 20, y: 20 });
  const [isDragging, setIsDragging] = createSignal<boolean>(false);
  
  const getActions = createMemo(() => {
    const instance: AssistiveButtonThis = {
      position,
      setPosition,
      isDragging,
      setIsDragging,
    }
    return {
      startDrag: startDrag.bind(instance),
      onDrag: onDrag.bind(instance),
      stopDrag: stopDrag.bind(instance),
    }
  })
  const getAbortController = createMemo(() => {
    return new AbortController()
  })

  const actions = getActions()
  const controller = getAbortController()
  const display = settingsPageVisibilitySignal[0] ? 'display:none' : ''
  const buttonStyle = 'background:#000000b3 z-index:1000 cursor:grab color:white border:none border-radius:50% position:fixed align-content:center'

  onMount(() => {
    window.addEventListener("mousemove", actions.onDrag as EventListener, { signal: controller.signal });
    window.addEventListener("touchmove", actions.onDrag as EventListener, { signal: controller.signal });
    window.addEventListener("mouseup", actions.stopDrag as EventListener, { signal: controller.signal });
    window.addEventListener("touchend", actions.stopDrag as EventListener, { signal: controller.signal });
  })

  onCleanup(() => {
    controller.abort()
  })

  return (
    <div class={`${display} top:${position().y}px left:${position().x}px w:${buttonSize}px h:${buttonSize}px ${buttonStyle}`}
        onMouseDown={actions.startDrag}
        onTouchStart={actions.startDrag}>
      Tap
    </div>
  );
}

export default AssistiveButton