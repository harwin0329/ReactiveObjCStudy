//
//  RACStream.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2012-10-31.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@class RACStream;

NS_ASSUME_NONNULL_BEGIN

/**
 An abstract class representing any stream of values.
 一个表示任何值流的抽象类。

 This class represents a monad, upon which many stream-based operations can be built.
 该类表示一个monad，可以构建许多基于流的操作。

 When subclassing RACStream, only the methods in the main @interface body need to be overridden.
 当子类化RACStream时，只需要重写主@interface主体中的方法。

 */
@interface RACStream<__covariant ValueType> : NSObject
// <__covariant ValueType> 的意思是： __covariant 协变，子类转父类；泛型名字是ValueType

    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wstrict-prototypes\"") \
    typedef id _Nonnull (^RACReduceBlock)();
    typedef ValueType _Nonnull (^RACGenericReduceBlock)();
_Pragma("clang diagnostic pop")
/**
 首先#pragma在本质上是声明，常用的功能就是注释，尤其是给Code分段注释；而且它还有另一个强大的功能是处理编译器警告 _Pragma() 是#pragma的另一种写法更加合理功能更多
 clang diagnostic 是#pragma 第一个常用命令：
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-相关命令"
 // 你自己的代码
 #pragma clang diagnostic pop

 这里的-Wstrict-prototypes 是警告： 函数声明不是一个原型
 函数声明 函数原型  还要多了解一下。
 总之这里就是忽略警告的意思
 */

/**
 抽象方法

 RACStream中的许多定义都是抽象的，没有具体实现，需要由其子类进行实现。
 */
// __kindof:表示当前类或者它子类

// Returns an empty stream.
// `因为RAC中nil会导致crash，所以很多时候需要定义一个空对象来替代nil，一般empty都被创建为一个单例来使用。 这个方法也是由子类来实现的`
+ (__kindof RACStream<ValueType> *)empty;

/*
 Lifts `value` into the stream monad.
 将“值”提升到monad流中。

 Returns a stream containing only the given value.
 返回只包含给定值的流。
*/
+ (__kindof RACStream<ValueType> *)return :(nullable ValueType)value;

/*
 A block which accepts a value from a RACStream and returns a new instance of the same stream class.
 一个block,从一个RACStream中接受一个值并返回同一stream类的新实例。

 Setting `stop` to `YES` will cause the bind to terminate after the returned value. Returning `nil` will result in immediate termination.
 将“stop”设置为“YES”将导致绑定（在返回值之后）终止。返回' nil '将导致立即终止。
*/
typedef RACStream *_Nullable (^RACStreamBindBlock)(ValueType _Nullable value, BOOL *stop);

/*
 Lazily binds a block to the values in the receiver.
 惰性地将一个块绑定到接收器中的值。

 This should only be used if you need to terminate the bind early, or close over some state. -flattenMap: is more appropriate for all other cases.
 只有在需要提前终止绑定或关闭某个状态时才应该使用此选项。flattenMap:更适合所有其他情况。

 block - A block returning a RACStreamBindBlock. This block will be invoked each time the bound stream is re-evaluated. This block must not be nil or return nil.
 一个返回RACStreamBindBlock的块。每当重新计算绑定流时，将调用此块。此块不能为nil或返回nil。

 Returns a new stream which represents the combined result of all lazy applications of `block`.
 返回一个新的流，它表示所有“块”的惰性应用程序的组合结果。
*/
- (__kindof RACStream *)bind:(RACStreamBindBlock (^)(void))block;

/*
 Appends the values of `stream` to the values in the receiver.
 将“stream”的值附加到接收器中的值。

 stream - A stream to concatenate. This must be an instance of the same concrete class as the receiver, and should not be `nil`.
 流-要连接的流。这必须是与接收者相同的具体类的实例，并且不应该是“nil”。

 Returns a new stream representing the receiver followed by `stream`.
 返回一个新的流，它表示后面跟着“流”的接收者。
*/
- (__kindof RACStream *)concat:(RACStream *)stream;

/*
 Zips the values in the receiver with those of the given stream to create RACTuples.

 The first value of each stream will be combined, then the second value, and so forth, until at least one of the streams is exhausted.

 stream - The stream to zip with. This must be an instance of the same concrete class as the receiver, and should not be `nil`.

 Returns a new stream of RACTuples, representing the zipped values of the two streams.

 将接收器中的值与给定流中的值进行压缩，以创建RACTuples。

 每个流的第一个值将被合并，然后是第二个值，依此类推，直到至少有一个流被耗尽。

 流-要用压缩的流。这必须是与接收者相同的具体类的实例，并且不应该是“nil”。

 返回一个新的RACTuples流，表示两个流的压缩值。
*/
- (__kindof RACStream *)zipWith:(RACStream *)stream;

@end

/*
 This extension contains functionality to support naming streams for debugging.

 Subclasses do not need to override the methods here.

 此扩展包含支持调试的命名流的功能。

 子类不需要在这里重写这些方法。
*/
@interface RACStream ()

// The name of the stream. This is for debugging/human purposes only.
@property (copy) NSString *name;

// Sets the name of the receiver to the given format string.
//
// This is for debugging purposes only, and won't do anything unless the
// RAC_DEBUG_SIGNAL_NAMES environment variable is set.
//
// Returns the receiver, for easy method chaining.
- (instancetype)setNameWithFormat:(NSString *)format, ... NS_FORMAT_FUNCTION(1, 2);

@end

/*
 Operations built on the RACStream primitives.

 These methods do not need to be overridden, although subclasses may occasionally gain better performance from doing so.

 在RACStream构建的操作。

 这些方法不需要重写，尽管子类偶尔可以通过这样做获得更好的性能。
*/
@interface RACStream<__covariant ValueType> (Operations)

/*
 Maps `block` across the values in the receiver and flattens the result.

 Note that operators applied _after_ -flattenMap: behave differently from operators _within_ -flattenMap:. See the Examples section below.

 This corresponds to the `SelectMany` method in Rx.

 block - A block which accepts the values in the receiver and returns a new instance of the receiver's class. Returning `nil` from this block is equivalent to returning an empty signal.

 Examples

 [signal flattenMap:^(id x) {
        // Logs each time a returned signal completes.
        //记录每次返回的信号完成时的日志。
        return [[RACSignal return:x] logCompleted];
 }];

 [[signal flattenMap:^(id x) {
        return [RACSignal return:x];
 }]
 // Logs only once, when all of the signals complete.
 //只记录一次，当所有的信号都完成时。
 logCompleted];

 Returns a new stream which represents the combined streams resulting from mapping `block`.

 在接收器的值之间映射“block”并使结果变平。

 注意，操作符应用_after_ -展平映射:与操作符_within_ -展平映射:的行为不同。参见下面的示例部分。

 这对应于Rx中的' SelectMany '方法。

 块-一个接受接收器中的值并返回接收器类的新实例的块。从这个块返回' nil '等同于返回一个空信号。

 返回一个新的流，它表示映射“block”产生的合并流。
*/
- (__kindof RACStream *)flattenMap:(__kindof RACStream *_Nullable (^)(ValueType _Nullable value))block;

/*
 Flattens a stream of streams.

 This corresponds to the `Merge` method in Rx.

 Returns a stream consisting of the combined streams obtained from the receiver.
 使一条小溪变平。

 这对应于Rx中的“Merge”方法。

 返回由从接收方获得的组合流组成的流。
*/
- (__kindof RACStream *)flatten;

/*
 Maps `block` across the values in the receiver.
 This corresponds to the `Select` method in Rx.
 Returns a new stream with the mapped values.

 在接收器中的值之间映射“块”。
 这对应于Rx中的' Select '方法。
 返回带有映射值的新流。
*/
- (__kindof RACStream *)map:(id _Nullable (^)(ValueType _Nullable value))block;

/*
 Replaces each value in the receiver with the given object.

 Returns a new stream which includes the given object once for each value in the receiver.

 将接收器中的每个值替换为给定的对象。

 返回一个新的流，该流为接收器中的每个值包含给定的对象一次。
*/
- (__kindof RACStream *)mapReplace:(nullable id)object;

/*
 Filters out values in the receiver that don't pass the given test.

 This corresponds to the `Where` method in Rx.

 Returns a new stream with only those values that passed.

 过滤掉接收器中不通过给定测试的值。

 这对应于Rx中的“Where”方法。

 返回一个新流，只包含那些已传递的值。
*/
- (__kindof RACStream<ValueType> *)filter:(BOOL (^)(ValueType _Nullable value))block;

/*
 Filters out values in the receiver that equal (via -isEqual:) the provided value.

 value - The value can be `nil`, in which case it ignores `nil` values.

 Returns a new stream containing only the values which did not compare equal to `value`.

 过滤接收方中与提供的值相等的值(通过-isEqual:)。

 值——值可以是“nil”，在这种情况下，它会忽略“nil”值。

 返回一个新流，其中只包含没有与“value”比较的值。
*/
- (__kindof RACStream<ValueType> *)ignore:(nullable ValueType)value;

/*
 Unpacks each RACTuple in the receiver and maps the values to a new value.

 reduceBlock - The block which reduces each RACTuple's values into one value. It must take as many arguments as the number of tuple elements to process. Each argument will be an object argument. The return value must be an object. This argument cannot be nil.

 Returns a new stream of reduced tuple values.

 解压接收器中的每个RACTuple并将值映射到一个新值。

 reduceBlock—将每个RACTuple值降低为一个值的块。它必须使用与要处理的元组元素数量相同的参数。每个参数都是一个对象参数。返回值必须是一个对象。这个参数不能为空。

 返回一个新的简化元组值流。
*/
- (__kindof RACStream *)reduceEach:(RACReduceBlock)reduceBlock;

/*
 Returns a stream consisting of `value`, followed by the values in the receiver.
 返回一个由“value”组成的流，后面跟着接收器中的值。
*/
- (__kindof RACStream<ValueType> *)startWith:(nullable ValueType)value;

/*
  Skips the first `skipCount` values in the receiver.

  Returns the receiver after skipping the first `skipCount` values. If `skipCount` is greater than the number of values in the stream, an empty stream is returned.

  跳过接收器中的最开始的' skipCount '值。

  跳过最开始的' skipCount '值后返回接收器。如果' skipCount '大于流中的值的数量，则返回一个空流。
 */
- (__kindof RACStream<ValueType> *)skip:(NSUInteger)skipCount;

/*
 Returns a stream of the first `count` values in the receiver. If `count` is greater than or equal to the number of values in the stream, a stream equivalent to the receiver is returned.
 返回接收器中第一个' count '值的流。如果“count”大于或等于流中的值的数量，则返回相当于接收方的流。
 */
- (__kindof RACStream<ValueType> *)take:(NSUInteger)count;

/*
 Zips the values in the given streams to create RACTuples.

 The first value of each stream will be combined, then the second value, and so forth, until at least one of the streams is exhausted.

 streams - The streams to combine. These must all be instances of the same concrete class implementing the protocol. If this collection is empty, the returned stream will be empty.

 Returns a new stream containing RACTuples of the zipped values from the
 streams.

 压缩给定流中的值以创建RACTuples。

 每个流的第一个值将被合并，然后是第二个值，依此类推，直到至少有一个流被耗尽。

 溪流-要结合的溪流。这些必须都是实现协议的同一个具体类的实例。如果此集合为空，则返回的流将为空。

 返回一个新的流，其中包含来自流的压缩值的RACTuples。
*/
+ (__kindof RACStream<ValueType> *)zip:(id<NSFastEnumeration>)streams;

/*
 Zips streams using +zip:, then reduces the resulting tuples into a single value using -reduceEach:

 streams     - The streams to combine. These must all be instances of the same concrete class implementing the protocol. If this collection is empty, the returned stream will be empty.
 reduceBlock - The block which reduces the values from all the streams into one value. It must take as many arguments as the number of streams given. Each argument will be an object argument. The return value must be an object. This argument must not be nil.

 Example:
 [RACStream zip:@[ stringSignal, intSignal ] reduce:^(NSString *string, NSNumber *number) {
 return [NSString stringWithFormat:@"%@: %@", string, number];
 }];

 Returns a new stream containing the results from each invocation of `reduceBlock`.
 使用+zip:压缩流，然后使用-reduceEach将产生的元组缩减成一个值:

 溪流-要结合的溪流。这些必须都是实现协议的同一个具体类的实例。如果此集合为空，则返回的流将为空。
 reduceBlock—将所有流中的值降低为一个值的块。它的参数必须与给定的流的数量相同。每个参数都是一个对象参数。返回值必须是一个对象。这个参数不可以是nil。

 返回一个新的流，其中包含每次调用“reduceBlock”的结果。
*/
+ (__kindof RACStream<ValueType> *)zip:(id<NSFastEnumeration>)streams reduce:(RACGenericReduceBlock)reduceBlock;

// Returns a stream obtained by concatenating `streams` in order.
//返回按顺序串联“streams”得到的流。
+ (__kindof RACStream<ValueType> *)concat:(id<NSFastEnumeration>)streams;

/*
 Combines values in the receiver from left to right using the given block.

 The algorithm proceeds as follows:

 1. `startingValue` is passed into the block as the `running` value, and the first element of the receiver is passed into the block as the `next` value.
 2. The result of the invocation is added to the returned stream.
 3. The result of the invocation (`running`) and the next element of the receiver (`next`) is passed into `block`.
 4. Steps 2 and 3 are repeated until all values have been processed.

 startingValue - The value to be combined with the first element of the receiver. This value may be `nil`.
 reduceBlock   - The block that describes how to combine values of the receiver. If the receiver is empty, this block will never be invoked. Cannot be nil.

 Examples
 RACSequence *numbers = @[ @1, @2, @3, @4 ].rac_sequence;

 // Contains 1, 3, 6, 10
 RACSequence *sums = [numbers scanWithStart:@0 reduce:^(NSNumber *sum, NSNumber *next) {
 return @(sum.integerValue + next.integerValue);
 }];

 Returns a new stream that consists of each application of `reduceBlock`. If the receiver is empty, an empty stream is returned.

 使用给定的块从左到右组合接收器中的值。

 算法流程如下:

 1. “startingValue”作为“running”值传递到块中，接收器的第一个元素作为“next”值传递到块中。
 2. 调用的结果被添加到返回的流中。
 3.调用的结果(“running”)和接收方的下一个元素(“next”)被传递到“block”中。
 4. 重复第2步和第3步，直到处理完所有值。

 startingValue—要与接收器的第一个元素结合的值。这个值可能是' nil '。
 reduceBlock—描述如何组合接收器的值的块。如果接收器为空，则永远不会调用此块。不能为零。

 返回一个由reduceBlock的每个应用程序组成的新流。如果接收方为空，则返回空流。
*/
- (__kindof RACStream *)scanWithStart:(nullable id)startingValue reduce:(id _Nullable (^)(id _Nullable running, ValueType _Nullable next))reduceBlock;

/*
 Combines values in the receiver from left to right using the given block which also takes zero-based index of the values.

 startingValue - The value to be combined with the first element of the receiver. This value may be `nil`.
 reduceBlock   - The block that describes how to combine values of the receiver. This block takes zero-based index value as the last parameter. If the receiver is empty, this block will never be invoked. Cannot be nil.

 Returns a new stream that consists of each application of `reduceBlock`. If the receiver is empty, an empty stream is returned.
 使用给定块从左到右组合接收器中的值，该块也接受值的从零开始的索引。

 startingValue—要与接收器的第一个元素结合的值。这个值可能是' nil '。
 reduceBlock—描述如何组合接收器的值的块。此块以从零开始的索引值作为最后一个参数。如果接收器为空，则永远不会调用此块。不能为零。

 返回一个由reduceBlock的每个应用程序组成的新流。如果接收方为空，则返回空流。
*/
- (__kindof RACStream *)scanWithStart:(nullable id)startingValue reduceWithIndex:(id _Nullable (^)(id _Nullable running, ValueType _Nullable next, NSUInteger index))reduceBlock;

/*
 Combines each previous and current value into one object.

 This method is similar to -scanWithStart:reduce:, but only ever operates on the previous and current values (instead of the whole stream), and does not
 pass the return value of `reduceBlock` into the next invocation of it.

 start       - The value passed into `reduceBlock` as `previous` for the first value.
 reduceBlock - The block that combines the previous value and the current value to create the reduced value. Cannot be nil.

 Examples

 RACSequence *numbers = @[ @1, @2, @3, @4 ].rac_sequence;

 // Contains 1, 3, 5, 7
 RACSequence *sums = [numbers combinePreviousWithStart:@0 reduce:^(NSNumber *previous, NSNumber *next) {
 return @(previous.integerValue + next.integerValue);
 }];

 Returns a new stream consisting of the return values from each application of `reduceBlock`.

 将以前的值和当前值合并到一个对象中。

 这个方法类似于-scanWithStart:reduce:，但是只对以前的和当前的值(而不是整个流)进行操作，而不进行操作
 将‘reduceBlock’的返回值传递到它的下一次调用中。

 start——第一个值作为“previous”传递给“reduceBlock”的值。
 reduceBlock—组合以前的值和当前值来创建减少的值的块。不能为零。

 返回一个新的流，该流由“reduceBlock”的每个应用程序的返回值组成。
*/
- (__kindof RACStream *)combinePreviousWithStart:(nullable ValueType)start reduce:(id _Nullable (^)(ValueType _Nullable previous, ValueType _Nullable current))reduceBlock;

/*
 Takes values until the given block returns `YES`.

 Returns a stream of the initial values in the receiver that fail `predicate`.
 If `predicate` never returns `YES`, a stream equivalent to the receiver is returned.

 获取值，直到给定的块返回“YES”。

 返回接收器中未通过“谓词”的初始值流。
 如果谓词从未返回“YES”，则返回与接收器等价的流。
*/
- (__kindof RACStream<ValueType> *)takeUntilBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/*
 Takes values until the given block returns `NO`.

 Returns a stream of the initial values in the receiver that pass `predicate`. If `predicate` never returns `NO`, a stream equivalent to the receiver is returned.

 在给定块返回“NO”之前接受值。

 返回接收器中传递谓词的初始值流。如果谓词从未返回“NO”，则返回与接收器等价的流。
*/
- (__kindof RACStream<ValueType> *)takeWhileBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/*
 Skips values until the given block returns `YES`.

 Returns a stream containing the values of the receiver that follow any initial values failing `predicate`. If `predicate` never returns `YES`, an empty stream is returned.

 跳过值，直到给定的块返回“YES”。

 返回一个流，其中包含接收者的值，该值跟随任何未通过“谓词”的初值。如果谓词从未返回“YES”，则返回一个空流。
*/
- (__kindof RACStream<ValueType> *)skipUntilBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/*
 Skips values until the given block returns `NO`.

 Returns a stream containing the values of the receiver that follow any initial values passing `predicate`. If `predicate` never returns `NO`, an empty stream is returned.

 跳过值，直到给定的块返回“NO”。

 返回一个包含接收者的值的流，这些值跟随任何传递“谓词”的初值。如果谓词从未返回NO，则返回一个空流。
*/
- (__kindof RACStream<ValueType> *)skipWhileBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/*
 Returns a stream of values for which - isEqual:returns NO when compared to the previous value.
 当RACStream的值和其上一个值比较不相等时，返回RACStream。（即发生了改变）
*/
- (__kindof RACStream<ValueType> *)distinctUntilChanged;

@end

NS_ASSUME_NONNULL_END
