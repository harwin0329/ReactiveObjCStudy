//
//  RACSignal.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 3/1/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RACAnnotations.h"
#import "RACStream.h"

@class RACDisposable;
@class RACScheduler;
@class RACSubject;
@class RACTuple;
@class RACTwoTuple<__covariant First, __covariant Second>;
@protocol RACSubscriber;

NS_ASSUME_NONNULL_BEGIN

@interface RACSignal<__covariant ValueType> : RACStream

/**
 Creates a new signal. This is the preferred way to create a new signal operation or behavior.

 Events can be sent to new subscribers immediately in the `didSubscribe` block, but the subscriber will not be able to dispose of the signal until a RACDisposable is returned from `didSubscribe`. In the case of infinite signals, this won't _ever_ happen if events are sent immediately.

 To ensure that the signal is disposable, events can be scheduled on the +[RACScheduler currentScheduler] (so that they're deferred, not sent immediately), or they can be sent in the background. The RACDisposable returned by the `didSubscribe` block should cancel any such scheduling or asynchronous work.

 didSubscribe - Called when the signal is subscribed to. The new subscriber is passed in. You can then manually control the <RACSubscriber> by sending it -sendNext:, -sendError:, and -sendCompleted, as defined by the operation you're implementing. This block should return a RACDisposable which cancels any ongoing work triggered by the subscription, and cleans up any resources or disposables created as part of it. When the disposable is disposed of, the signal must not send any more events to the `subscriber`. If no cleanup is necessary, return nil.

 **Note:** The `didSubscribe` block is called every time a new subscriber subscribes. Any side effects within the block will thus execute once for each subscription, not necessarily on one thread, and possibly even simultaneously!

 创建一个新信号。这是创建新信号操作或行为的首选方法。
 
 事件可以立即发送到“didSubscribe”块中的新订阅者，但是订阅者在从“didSubscribe”返回RACDisposable之前无法处理信号。在无限信号的情况下，如果事件被立即发送，这将不会永远发生。
 
 为了确保信号是可丢弃的，可以在+[RACScheduler currentScheduler]上调度事件(以便延迟发送，而不是立即发送)，也可以在后台发送事件。由' didSubscribe '块返回的RACDisposable应该取消任何此类调度或异步工作。
 
 didSubscribe -当信号被订阅时调用。新订户被传入。然后，您可以通过发送-sendNext:、-sendError:和-sendCompleted来手动控制<RACSubscriber>，这是您正在实现的操作所定义的。此块应返回一个RACDisposable，它取消订阅所触发的任何正在进行的工作，并清理作为其一部分创建的任何资源或一次性用品。当丢弃处理后，信号不能再向“订阅方”发送任何事件。如果不需要清除，返回nil。
 
 **注意:**每次新订户订阅时都会调用' didSubscribe '块。因此，块内的任何副作用将对每个订阅执行一次，不一定是在一个线程上，甚至可能是同时执行!
 */
+ (RACSignal<ValueType> *)createSignal:(RACDisposable *_Nullable (^)(id<RACSubscriber> subscriber))didSubscribe RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal that immediately sends the given error.
 返回立即发送给定错误的信号。
*/
+ (RACSignal<ValueType> *)error:(nullable NSError *)error RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal that never completes.

 返回一个永远不会完成的信号。
*/
+ (RACSignal<ValueType> *)never RAC_WARN_UNUSED_RESULT;

/*
 Immediately schedules the given block on the given scheduler. The block is given a subscriber to which it can send events.

 scheduler - The scheduler on which `block` will be scheduled and results delivered. Cannot be nil.
 block     - The block to invoke. Cannot be NULL.

 Returns a signal which will send all events sent on the subscriber given to `block`. All events will be sent on `scheduler` and it will replay any missed events to new subscribers.

 立即在给定的调度程序上调度给定的块。块被赋予一个订阅者，它可以向这个订阅者发送事件。

 调度程序-调度程序上的“块”将被调度和结果交付。不能为零。
 块-要调用的块。不能为空。

 返回一个信号，该信号将发送所有发送到订阅服务器的事件。所有事件将发送'调度'，它将重播任何错过的事件，新的订户。
*/
+ (RACSignal<ValueType> *)startEagerlyWithScheduler:(RACScheduler *)scheduler block:(void (^)(id<RACSubscriber> subscriber))block;

/*
 Invokes the given block only on the first subscription. The block is given a subscriber to which it can send events.

 Note that disposing of the subscription to the returned signal will *not* dispose of the underlying subscription. If you need that behavior, see -[RACMulticastConnection autoconnect]. The underlying subscription will never be disposed of. Because of this, `block` should never return an infinite signal since there would be no way of ending it.

 scheduler - The scheduler on which the block should be scheduled. Note that if given +[RACScheduler immediateScheduler], the block will be invoked synchronously on the first subscription. Cannot be nil.
 block     - The block to invoke on the first subscription. Cannot be NULL.

 Returns a signal which will pass through the events sent to the subscriber given to `block` and replay any missed events to new subscribers.

 仅在第一次订阅时调用给定块。块被赋予一个订阅者，它可以向这个订阅者发送事件。

 注意，将订阅处理为返回信号将“不”处理底层订阅。如果您需要该行为，请参见-[RACMulticastConnection autoconnect]。底层订阅将永远不会被释放。因此，“block”永远不应该返回一个无限的信号，因为没有办法结束它。

 调度程序——应该调度块的调度程序。注意，如果给定+[RACScheduler immediate ateschscheduler]，则将在第一次订阅时同步调用该块。不能为零。
 块——在第一次订阅时要调用的块。不能为空。

 返回一个信号，该信号将通过发送给订阅方的事件进行“阻止”，并将任何错过的事件重新发送给新订阅方。
*/
+ (RACSignal<ValueType> *)startLazilyWithScheduler:(RACScheduler *)scheduler block:(void (^)(id<RACSubscriber> subscriber))block RAC_WARN_UNUSED_RESULT;

@end

@interface RACSignal<__covariant ValueType> (RACStream)

/*
 Returns a signal that immediately sends the given value and then completes.

 返回一个信号，该信号立即发送给定的值，然后完成。
*/
+ (RACSignal<ValueType> *)return :(nullable ValueType)value RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal that immediately completes.

 返回一个立即完成的信号。
*/
+ (RACSignal<ValueType> *)empty RAC_WARN_UNUSED_RESULT;

/*
 A block which accepts a value from a RACSignal and returns a new signal.
 Setting `stop` to `YES` will cause the bind to terminate after the returned value. Returning `nil` will result in immediate termination.

 从RACSignal中接受一个值并返回一个新信号的块。
 将“stop”设置为“YES”将导致绑定在返回值之后终止。返回' nil '将导致立即终止。
*/
typedef RACSignal *_Nullable (^RACSignalBindBlock)(ValueType _Nullable value, BOOL *stop);

/*
 Lazily binds a block to the values in the receiver.

 This should only be used if you need to terminate the bind early, or close over some state. -flattenMap: is more appropriate for all other cases.

 block - A block returning a RACSignalBindBlock. This block will be invoked each time the bound signal is re-evaluated. This block must not be nil or return nil.

 Returns a new signal which represents the combined result of all lazy
 applications of `block`.

 惰性地将一个块绑定到接收器中的值。
 只有在需要提前终止绑定或关闭某个状态时才应该使用此选项。-展平图:更适合所有其他情况。
 一个返回RACSignalBindBlock的块。每当重新计算绑定信号时将调用此块。此块不能为nil或返回nil。
 返回一个新信号，该信号表示所有惰性操作的组合结果
 “块”的应用程序。
*/
- (RACSignal *)bind:(RACSignalBindBlock (^)(void))block RAC_WARN_UNUSED_RESULT;

/*
 Subscribes to `signal` when the source signal completes.
 当源信号完成时订阅“signal”
*/
- (RACSignal *)concat:(RACSignal *)signal RAC_WARN_UNUSED_RESULT;

/*
 Zips the values in the receiver with those of the given signal to create RACTuples.

 The first `next` of each signal will be combined, then the second `next`, and so forth, until either signal completes or errors.

 signal - The signal to zip with. This must not be `nil`.

 Returns a new signal of RACTuples, representing the combined values of the two signals. Any error from one of the original signals will be forwarded on the returned signal.

 将接收器中的值与给定信号中的值进行压缩，以创建RACTuples。
 每个信号的第一个“next”将被合并，然后是第二个“next”，以此类推，直到信号完成或出现错误。
 信号-信号zip与。这个不能是nil。
 返回一个RACTuples的新信号，表示两个信号的组合值。任何来自原始信号的错误都将在返回的信号上转发。
*/
- (RACSignal<RACTwoTuple<ValueType, id> *> *)zipWith:(RACSignal *)signal RAC_WARN_UNUSED_RESULT;

@end

/*
 Redeclarations of operations built on the RACStream primitives with more precise ValueType information.

 In cases where the ValueType of the result of the operation is not able to be inferred, the ValueType is erased in the result.

 In cases where instancetype is a valid return type, the operation is not redeclared here.

 使用更精确的ValueType信息在RACStream原语上构建的操作的重新声明。

 在操作的结果的ValueType不能被推断的情况下，ValueType在结果中被擦除。

 在instancetype是有效返回类型的情况下，这里不会重新声明操作。
*/
@interface RACSignal<__covariant ValueType> (RACStreamOperations)

/*
 Maps `block` across the values in the receiver and flattens the result.

 Note that operators applied _after_ -flattenMap: behave differently from operators _within_ -flattenMap:. See the Examples section below.

 This corresponds to the `SelectMany` method in Rx.

 block - A block which accepts the values in the receiver and returns a new instance of the receiver's class. Returning `nil` from this block is equivalent to returning an empty signal.

 Examples

 [signal flattenMap:^(id x) {
 // Logs each time a returned signal completes.
 return [[RACSignal return:x] logCompleted];
 }];

 [[signal flattenMap:^(id x) {
 return [RACSignal return:x];
 }]
 // Logs only once, when all of the signals complete.
 logCompleted];

 Returns a new signal which represents the combined signals resulting from mapping `block`.

 在接收器的值之间映射“block”并使结果变平。

 注意，操作符应用_after_ -展平映射:与操作符_within_ -展平映射:的行为不同。参见下面的示例部分。

 这对应于Rx中的' SelectMany '方法。

 块-一个接受接收器中的值并返回接收器类的新实例的块。从这个块返回' nil '等同于返回一个空信号。

 例子

 [信号展平图:^(idx) {
 //记录每次返回的信号完成时的日志。
 return [[RACSignal return:x] logCompleted];
 });

 [[信号展平图:^(idx) {]
 返回[RACSignal返回x):;
 })
 //只记录一次，当所有的信号都完成时。
 logCompleted];

 返回一个新的信号，它表示映射“block”产生的组合信号。
*/
- (RACSignal *)flattenMap:(__kindof RACSignal *_Nullable (^)(ValueType _Nullable value))block RAC_WARN_UNUSED_RESULT;

/*
 Flattens a signal of signals.

 This corresponds to the `Merge` method in Rx.

 Returns a signal consisting of the combined signals obtained from the receiver.

 展平信号的信号。

 这对应于Rx中的“Merge”方法。

 返回由从接收机获得的组合信号组成的信号。
*/
- (RACSignal *)flatten RAC_WARN_UNUSED_RESULT;

/*
 Maps `block` across the values in the receiver.

 This corresponds to the `Select` method in Rx.

 Returns a new signal with the mapped values.

 在接收器中的值之间映射“块”。

 这对应于Rx中的' Select '方法。

 返回带有映射值的新信号。
*/
- (RACSignal *)map:(id _Nullable (^)(ValueType _Nullable value))block RAC_WARN_UNUSED_RESULT;

/*
 Replaces each value in the receiver with the given object.

 Returns a new signal which includes the given object once for each value in the receiver.

 将接收器中的每个值替换为给定的对象。

 返回一个新信号，该信号为接收器中的每个值包含给定的对象一次。
*/
- (RACSignal *)mapReplace:(nullable id)object RAC_WARN_UNUSED_RESULT;

/*
 Filters out values in the receiver that don't pass the given test.

 This corresponds to the `Where` method in Rx.

 Returns a new signal with only those values that passed.

 过滤掉接收器中不通过给定测试的值。

 这对应于Rx中的“Where”方法。

 返回一个新信号，仅包含那些已传递的值。
*/
- (RACSignal<ValueType> *)filter:(BOOL (^)(ValueType _Nullable value))block RAC_WARN_UNUSED_RESULT;

/*
 Filters out values in the receiver that equal (via -isEqual:) the provided value.

 value - The value can be `nil`, in which case it ignores `nil` values.

 Returns a new signal containing only the values which did not compare equal to `value`.

 过滤接收方中与提供的值相等的值(通过-isEqual:)。

 值——值可以是“nil”，在这种情况下，它会忽略“nil”值。

 返回一个新信号，该信号只包含没有与“value”比较的值。
*/
- (RACSignal<ValueType> *)ignore:(nullable ValueType)value RAC_WARN_UNUSED_RESULT;

/*
 Unpacks each RACTuple in the receiver and maps the values to a new value.

 reduceBlock - The block which reduces each RACTuple's values into one value. It must take as many arguments as the number of tuple elements to process. Each argument will be an object argument. The return value must be an object. This argument cannot be nil.

 Returns a new signal of reduced tuple values.

 解压接收器中的每个RACTuple并将值映射到一个新值。

 reduceBlock—将每个RACTuple值降低为一个值的块。它必须使用与要处理的元组元素数量相同的参数。每个参数都是一个对象参数。返回值必须是一个对象。这个参数不能为空。

 返回一个新的缩减元组值的信号。
*/
- (RACSignal *)reduceEach:(RACReduceBlock)reduceBlock RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal consisting of `value`, followed by the values in the receiver.

 返回一个由“值”组成的信号，后面跟着接收器中的值。
*/
- (RACSignal<ValueType> *)startWith:(nullable ValueType)value RAC_WARN_UNUSED_RESULT;

/*
 Skips the first `skipCount` values in the receiver.

 Returns the receiver after skipping the first `skipCount` values. If `skipCount` is greater than the number of values in the signal, an empty signal is returned.

 跳过接收器中的第一个' skipCount '值。

 跳过第一个' skipCount '值后返回接收器。如果' skipCount '大于信号中的值的数量，则返回一个空信号。
*/
- (RACSignal<ValueType> *)skip:(NSUInteger)skipCount RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal of the first `count` values in the receiver. If `count` is greater than or equal to the number of values in the signal, a signal equivalent to the receiver is returned.

 返回接收器中第一个' count '值的信号。如果“count”大于或等于信号中的值的数量，则返回与接收端相等的信号。
*/
- (RACSignal<ValueType> *)take:(NSUInteger)count RAC_WARN_UNUSED_RESULT;

/*
 Zips the values in the given signals to create RACTuples.

 The first value of each signals will be combined, then the second value, and so forth, until at least one of the signals is exhausted.

 signals - The signals to combine. If this collection is empty, the returned signal will be empty.

 Returns a new signal containing RACTuples of the zipped values from the signals.

 压缩给定信号中的值以创建RACTuples。

 每个信号的第一个值将被合并，然后是第二个值，依此类推，直到至少一个信号被耗尽。

 信号-组合的信号。如果此集合为空，则返回的信号将为空。

 返回一个新信号，其中包含来自信号的压缩值的RACTuples。
*/
+ (RACSignal<RACTuple *> *)zip:(id<NSFastEnumeration>)signals RAC_WARN_UNUSED_RESULT;

/*
 Zips signals using +zip:, then reduces the resulting tuples into a single value using -reduceEach:

 signals     - The signals to combine. If this collection is empty, the returned signal will be empty.
 reduceBlock - The block which reduces the values from all the signals into one value. It must take as many arguments as the number of signals given. Each argument will be an object argument. The return value must be an object. This argument must not be nil.

 Example:

 [RACSignal zip:@[ stringSignal, intSignal ]
 reduce:^(NSString *string, NSNumber *number) {
 return [NSString stringWithFormat:@"%@: %@", string, number];
 }];

 Returns a new signal containing the results from each invocation of `reduceBlock`.

 使用+zip:对信号进行压缩，然后使用-reduceEach将产生的元组缩减为单个值:

 信号-组合的信号。如果此集合为空，则返回的信号将为空。
 reduceBlock—将所有信号的值降低为一个值的块。它的参数必须和给出的信号数量一样多。每个参数都是一个对象参数。返回值必须是一个对象。这个参数不可以是nil。

 返回一个新信号，其中包含每次调用“reduceBlock”的结果。
*/
+ (RACSignal<ValueType> *)zip:(id<NSFastEnumeration>)signals reduce:(RACGenericReduceBlock)reduceBlock RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal obtained by concatenating `signals` in order.

 返回按顺序连接“信号”得到的信号。
*/
+ (RACSignal<ValueType> *)concat:(id<NSFastEnumeration>)signals RAC_WARN_UNUSED_RESULT;

/*
 Combines values in the receiver from left to right using the given block.

 The algorithm proceeds as follows:
 1. `startingValue` is passed into the block as the `running` value, and the first element of the receiver is passed into the block as the `next` value.
 2. The result of the invocation is added to the returned signal.
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

 Returns a new signal that consists of each application of `reduceBlock`. If the receiver is empty, an empty signal is returned.

 使用给定的块从左到右组合接收器中的值。

 算法流程如下:
 1. “startingValue”作为“running”值传递到块中，接收器的第一个元素作为“next”值传递到块中。
 2. 调用的结果被添加到返回的信号中。
 3.调用的结果(“running”)和接收方的下一个元素(“next”)被传递到“block”中。
 4. 重复第2步和第3步，直到处理完所有值。

 startingValue—要与接收器的第一个元素结合的值。这个值可能是' nil '。
 reduceBlock—描述如何组合接收器的值的块。如果接收器为空，则永远不会调用此块。不能为零。

 返回一个由“reduceBlock”的每个应用程序组成的新信号。如果接收器为空，则返回空信号。
*/
- (RACSignal *)scanWithStart:(nullable id)startingValue reduce:(id _Nullable (^)(id _Nullable running, ValueType _Nullable next))reduceBlock RAC_WARN_UNUSED_RESULT;

/*
 Combines values in the receiver from left to right using the given block which also takes zero-based index of the values.

 startingValue - The value to be combined with the first element of the receiver. This value may be `nil`.
 reduceBlock   - The block that describes how to combine values of the receiver. This block takes zero-based index value as the last parameter. If the receiver is empty, this block will never be invoked. Cannot be nil.

 Returns a new signal that consists of each application of `reduceBlock`. If the receiver is empty, an empty signal is returned.
 使用给定块从左到右组合接收器中的值，该块也接受值的从零开始的索引。

 startingValue—要与接收器的第一个元素结合的值。这个值可能是' nil '。
 reduceBlock—描述如何组合接收器的值的块。此块以从零开始的索引值作为最后一个参数。如果接收器为空，则永远不会调用此块。不能为零。

 返回一个由“reduceBlock”的每个应用程序组成的新信号。如果接收器为空，则返回空信号。
*/
- (RACSignal *)scanWithStart:(nullable id)startingValue reduceWithIndex:(id _Nullable (^)(id _Nullable running, ValueType _Nullable next, NSUInteger index))reduceBlock RAC_WARN_UNUSED_RESULT;

/*
 Combines each previous and current value into one object.

 This method is similar to -scanWithStart:reduce:, but only ever operates on the previous and current values (instead of the whole signal), and does not pass the return value of `reduceBlock` into the next invocation of it.

 start       - The value passed into `reduceBlock` as `previous` for the first value.
 reduceBlock - The block that combines the previous value and the current value to create the reduced value. Cannot be nil.

 Examples

     RACSignal<NSNumber *> *numbers = [@[ @1, @2, @3, @4 ].rac_sequence
         signalWithScheduler:RACScheduler.immediateScheduler];

     // Contains 1, 3, 5, 7
     RACSignal *sums = [numbers combinePreviousWithStart:@0 reduce:^(NSNumber *previous, NSNumber *next) {
         return @(previous.integerValue + next.integerValue);
     }];

 Returns a new signal consisting of the return values from each application of `reduceBlock`.
 将以前的值和当前值合并到一个对象中。

 这个方法类似于-scanWithStart:reduce:，但是只对前一个和当前的值(而不是整个信号)进行操作，并且不将‘reduceBlock’的返回值传递到下一次对它的调用中。

 start——第一个值作为“previous”传递给“reduceBlock”的值。
 reduceBlock—组合以前的值和当前值来创建减少的值的块。不能为零。

 返回一个新的信号，该信号由“reduceBlock”的每个应用程序的返回值组成。
*/
- (RACSignal *)combinePreviousWithStart:(nullable ValueType)start reduce:(id _Nullable (^)(ValueType _Nullable previous, ValueType _Nullable current))reduceBlock RAC_WARN_UNUSED_RESULT;

/*
 Takes values until the given block returns `YES`.
 Returns a signal of the initial values in the receiver that fail `predicate`. If `predicate` never returns `YES`, a signal equivalent to the receiver is returned.

 获取值，直到给定的块返回“YES”。
 返回接收器中未通过谓词的初始值的信号。如果谓词从未返回“YES”，则返回一个与接收器等价的信号。
*/
- (RACSignal<ValueType> *)takeUntilBlock:(BOOL (^)(ValueType _Nullable x))predicate RAC_WARN_UNUSED_RESULT;

/*
 Takes values until the given block returns `NO`.
 Returns a signal of the initial values in the receiver that pass `predicate`. If `predicate` never returns `NO`, a signal equivalent to the receiver is returned.
 
 在给定块返回“NO”之前接受值。
 返回接收器中传递谓词的初始值的信号。如果谓词从未返回“NO”，则返回一个与接收器等价的信号。
*/
- (RACSignal<ValueType> *)takeWhileBlock:(BOOL (^)(ValueType _Nullable x))predicate RAC_WARN_UNUSED_RESULT;

/*
 Skips values until the given block returns `YES`.
 Returns a signal containing the values of the receiver that follow any initial values failing `predicate`. If `predicate` never returns `YES`, an empty signal is returned.

 跳过值，直到给定的块返回“YES”。
 返回一个信号，该信号包含接收器的值，该值跟随任何未通过谓词的初值。如果谓词从未返回“YES”，则返回一个空信号。
*/
- (RACSignal<ValueType> *)skipUntilBlock:(BOOL (^)(ValueType _Nullable x))predicate RAC_WARN_UNUSED_RESULT;

/*
 Skips values until the given block returns `NO`.
 Returns a signal containing the values of the receiver that follow any initial values passing `predicate`. If `predicate` never returns `NO`, an empty signal is returned.

 跳过值，直到给定的块返回“NO”。
 返回一个信号，该信号包含接收器的值，该值跟随任何传递“谓词”的初值。如果谓词从未返回NO，则返回一个空信号。
*/
- (RACSignal<ValueType> *)skipWhileBlock:(BOOL (^)(ValueType _Nullable x))predicate RAC_WARN_UNUSED_RESULT;

/*
 Returns a signal of values for which -isEqual: returns NO when compared to the previous value.
 当与前一个值相比等于NO时返回信号。
*/
- (RACSignal<ValueType> *)distinctUntilChanged RAC_WARN_UNUSED_RESULT;

@end

@interface RACSignal<__covariant ValueType> (Subscription)

/*
 Subscribes `subscriber` to changes on the receiver. The receiver defines which events it actually sends and in what situations the events are sent.

 Subscription will always happen on a valid RACScheduler. If the +[RACScheduler currentScheduler] cannot be determined at the time of subscription (e.g., because the calling code is running on a GCD queue or NSOperationQueue), subscription will occur on a private background scheduler.
 On the main thread, subscriptions will always occur immediately, with a +[RACScheduler currentScheduler] of +[RACScheduler mainThreadScheduler].

 This method must be overridden by any subclasses.

 Returns nil or a disposable. You can call -[RACDisposable dispose] if you need to end your subscription before it would "naturally" end, either by completing or erroring. Once the disposable has been disposed, the subscriber won't receive any more events from the subscription.

 订阅“订阅方”以更改接收方。接收方定义它实际发送的事件以及在什么情况下发送事件。

 订阅将始终在有效的RACScheduler上发生。如果在订阅时无法确定+[RACScheduler currentScheduler](例如，因为调用代码在GCD队列或NSOperationQueue上运行)，订阅将在私有后台调度程序上发生。
 在主线程上，订阅总是立即发生，使用+[RACScheduler currentScheduler]的+[RACScheduler mainThreadScheduler]。

 这个方法必须被任何子类覆盖。

 返回nil或一次性的。您可以调用-[RACDisposable dispose]，如果您需要在订阅“自然”结束之前结束订阅，无论是通过完成还是出错。一旦丢弃丢弃，订阅者将不再从订阅中接收任何事件。
*/
- (RACDisposable *)subscribe:(id<RACSubscriber>)subscriber;

/*
 Convenience method to subscribe to the `next` event.
 This corresponds to `IObserver<T>.OnNext` in Rx.

 订阅“next”事件的便利方法。
 这对应于IObserver<T>。在Rx OnNext”。
*/
- (RACDisposable *)subscribeNext:(void (^)(ValueType _Nullable x))nextBlock;

/*
 Convenience method to subscribe to the `next` and `completed` events.
 订阅“next”和“completed”事件的便利方法。
*/
- (RACDisposable *)subscribeNext:(void (^)(ValueType _Nullable x))nextBlock completed:(void (^)(void))completedBlock;

/*
 Convenience method to subscribe to the `next`, `completed`, and `error` events.
 订阅“next”、“completed”和“error”事件的便利方法。
*/
- (RACDisposable *)subscribeNext:(void (^)(ValueType _Nullable x))nextBlock error:(void (^)(NSError *_Nullable error))errorBlock completed:(void (^)(void))completedBlock;

/*
 Convenience method to subscribe to `error` events.
 This corresponds to the `IObserver<T>.OnError` in Rx.

 订阅“错误”事件的便利方法。
 这对应于IObserver<T>。在Rx OnError”。
*/
- (RACDisposable *)subscribeError:(void (^)(NSError *_Nullable error))errorBlock;

/*
 Convenience method to subscribe to `completed` events.
 This corresponds to the `IObserver<T>.OnCompleted` in Rx.

 订阅“已完成”事件的便利方法。
 这对应于IObserver<T>。在Rx oncomplete”。
*/
- (RACDisposable *)subscribeCompleted:(void (^)(void))completedBlock;

/*
 Convenience method to subscribe to `next` and `error` events.
 订阅“next”和“error”事件的便利方法。
*/
- (RACDisposable *)subscribeNext:(void (^)(ValueType _Nullable x))nextBlock error:(void (^)(NSError *_Nullable error))errorBlock;

/*
 Convenience method to subscribe to `error` and `completed` events.
 订阅“错误”和“完成”事件的便利方法。
*/
- (RACDisposable *)subscribeError:(void (^)(NSError *_Nullable error))errorBlock completed:(void (^)(void))completedBlock;

@end

// Additional methods to assist with debugging.
@interface RACSignal<__covariant ValueType> (Debugging)

// Logs all events that the receiver sends.
- (RACSignal<ValueType> *)logAll RAC_WARN_UNUSED_RESULT;

// Logs each `next` that the receiver sends.
- (RACSignal<ValueType> *)logNext RAC_WARN_UNUSED_RESULT;

// Logs any error that the receiver sends.
- (RACSignal<ValueType> *)logError RAC_WARN_UNUSED_RESULT;

// Logs any `completed` event that the receiver sends.
- (RACSignal<ValueType> *)logCompleted RAC_WARN_UNUSED_RESULT;

@end

// Additional methods to assist with unit testing.
//
// **These methods should never ship in production code.**
@interface RACSignal<__covariant ValueType> (Testing)

// Spins the main run loop for a short while, waiting for the receiver to send a `next` or the provided timeout to elapse.
//
// **Because this method executes the run loop recursively, it should only be used on the main thread, and only from a unit test.**
//
// defaultValue - Returned if the receiver completes or errors before sending a `next`, or if the method times out. This argument may be nil.
// success      - If not NULL, set to whether the receiver completed successfully.
// error        - If not NULL, set to any error that occurred.
//
// Returns the first value received, or `defaultValue` if no value is received
// before the signal finishes or the method times out.
- (nullable ValueType)asynchronousFirstOrDefault:(nullable ValueType)defaultValue success:(nullable BOOL *)success error:(NSError *_Nullable *_Nullable)error timeout:(NSTimeInterval)timeout;

// Spins the main run loop for a short while, waiting for the receiver to send a `next`.
//
// **Because this method executes the run loop recursively, it should only be used on the main thread, and only from a unit test.**
//
// defaultValue - Returned if the receiver completes or errors before sending a `next`, or if the method times out. This argument may be nil.
// success      - If not NULL, set to whether the receiver completed successfully.
// error        - If not NULL, set to any error that occurred.
//
// Returns the first value received, or `defaultValue` if no value is received
// before the signal finishes or the method times out.
- (nullable ValueType)asynchronousFirstOrDefault:(nullable ValueType)defaultValue success:(nullable BOOL *)success error:(NSError *_Nullable *_Nullable)error;

// Spins the main run loop for a short while, waiting for the receiver to complete. or the provided timeout to elapse.
//
// **Because this method executes the run loop recursively, it should only be used on the main thread, and only from a unit test.**
//
// error - If not NULL, set to any error that occurs.
//
// Returns whether the signal completed successfully before timing out. If NO, `error` will be set to any error that occurred.
- (BOOL)asynchronouslyWaitUntilCompleted:(NSError *_Nullable *_Nullable)error timeout:(NSTimeInterval)timeout;

// Spins the main run loop for a short while, waiting for the receiver to complete
//
// **Because this method executes the run loop recursively, it should only be used on the main thread, and only from a unit test.**
//
// error - If not NULL, set to any error that occurs.
//
// Returns whether the signal completed successfully before timing out. If NO, `error` will be set to any error that occurred.
- (BOOL)asynchronouslyWaitUntilCompleted:(NSError *_Nullable *_Nullable)error;

@end

NS_ASSUME_NONNULL_END
