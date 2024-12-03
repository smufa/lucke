package circularbuffer

import (
	"encoding/json"
	"fmt"
)

// CircularBuffer holds a fixed-size buffer with a pointer for the next write position.
type CircularBuffer struct {
	buffer []int64
	size   int
	head   int
	full   bool
}

// NewCircularBuffer creates a new CircularBuffer of the given size.
func NewCircularBuffer(size int) *CircularBuffer {
	return &CircularBuffer{
		buffer: make([]int64, size),
		size:   size,
		head:   0,
		full:   false,
	}
}

// Add adds a new value to the buffer, overwriting the oldest value if the buffer is full.
func (cb *CircularBuffer) Add(value int64) *CircularBuffer {
	cb.buffer[cb.head] = value
	cb.head = (cb.head + 1) % cb.size
	if cb.head == 0 {
		cb.full = true
	}
	return cb
}

// Values retrieves the buffer's contents in the correct order.
func (cb *CircularBuffer) Values() []int64 {
	if !cb.full {
		return cb.buffer[:cb.head]
	}
	// If full, return values in circular order.
	return append(cb.buffer[cb.head:], cb.buffer[:cb.head]...)
}

func NewCircularBufferWithVals(size int, value int64) *CircularBuffer {
	buff := &CircularBuffer{
		buffer: make([]int64, size),
		size:   size,
		head:   0,
		full:   false,
	}

	buff.Add(value)

	return buff
}

// MarshalJSON defines how the CircularBuffer is converted to JSON.
func (cb *CircularBuffer) MarshalJSON() ([]byte, error) {
	return json.Marshal(cb.Values())
}

// UnmarshalJSON defines how the CircularBuffer is restored from JSON.
func (cb *CircularBuffer) UnmarshalJSON(data []byte) error {
	var values []int64
	if err := json.Unmarshal(data, &values); err != nil {
		return err
	}

	// Reconstruct the circular buffer
	cb.size = len(values)
	cb.buffer = make([]int64, cb.size)
	copy(cb.buffer, values)
	cb.head = 0
	cb.full = true

	return nil
}

// String makes CircularBuffer implement the fmt.Stringer interface.
// It returns the buffer's values as a string.
func (cb *CircularBuffer) String() string {
	return fmt.Sprintf("%v", cb.Values())
}
