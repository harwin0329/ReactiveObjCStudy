//
//  RACSignal.m
//  ReactiveObjC
//
//  Created by Josh Abernathy on 3/15/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import "RACSignal.h"
#import "RACCompoundDisposable.h"
#import "RACDisposable.h"
#import "RACDynamicSignal.h"
#import "RACEmptySignal.h"
#import "RACErrorSignal.h"
#import "RACMulticastConnection.h"
#import "RACReplaySubject.h"
#import "RACReturnSignal.h"
#import "RACScheduler.h"
#import "RACSerialDisposable.h"
#import "RACSignal+Operations.h"
#import "RACSubject.h"
#import "RACSubscriber+Private.h"
#import "RACTuple.h"
#import <libkern/OSAtomic.h>

@implementation RACSignal

#pragma mark Lifecycle

// 创建信号（挪步至 子类RACDynamicSignal）
+ (RACSignal *)createSignal:(RACDisposable * (^)(id<RACSubscriber> subscriber))didSubscribe {
    return [RACDynamicSignal createSignal:didSubscribe];
}

+ (RACSignal *)error:(NSError *)error {
    return [RACErrorSignal error:error];
}

+ (RACSignal *)never {
    return [[self createSignal:^ RACDisposable * (id<RACSubscriber> subscriber) {
        return nil;
    }] setNameWithFormat:@"+never"];
}

+ (RACSignal *)startEagerlyWithScheduler:(RACScheduler *)scheduler block:(void (^)(id<RACSubscriber> subscriber))block {
    NSCParameterAssert(scheduler != nil);
    NSCParameterAssert(block != NULL);
    
    RACSignal *signal = [self startLazilyWithScheduler:scheduler block:block];
    // Subscribe to force the lazy signal to call its block.
    [[signal publish] connect];
    return [signal setNameWithFormat:@"+startEagerlyWithScheduler: %@ block:", scheduler];
}

+ (RACSignal *)startLazilyWithScheduler:(RACScheduler *)scheduler block:(void (^)(id<RACSubscriber> subscriber))block {
    NSCParameterAssert(scheduler != nil);
    NSCParameterAssert(block != NULL);
    
    RACMulticastConnection *connection = [[RACSignal
                                           createSignal:^ id (id<RACSubscriber> subscriber) {
        block(subscriber);
        return nil;
    }]
                                          multicast:[RACReplaySubject subject]];
    
    return [[[RACSignal
              createSignal:^ id (id<RACSubscriber> subscriber) {
        [connection.signal subscribe:subscriber];
        [connection connect];
        return nil;
    }]
             subscribeOn:scheduler]
            setNameWithFormat:@"+startLazilyWithScheduler: %@ block:", scheduler];
}

#pragma mark NSObject

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p> name: %@", self.class, self, self.name];
}

@end

@implementation RACSignal (RACStream)

+ (RACSignal *)empty {
    return [RACEmptySignal empty];
}

+ (RACSignal *)return:(id)value {
    return [RACReturnSignal return:value];
}

- (RACSignal *)bind:(RACSignalBindBlock (^)(void))block {
    NSCParameterAssert(block != NULL);
    /*
     -bind: should:
     
     1. Subscribe to the original signal of values.
     2. Any time the original signal sends a value, transform it using the binding block.
     3. If the binding block returns a signal, subscribe to it, and pass all of its values through to the subscriber as they're received.
     4. If the binding block asks the bind to terminate, complete the _original_ signal.
     5. When _all_ signals complete, send completed to the subscriber.
     
     If any signal sends an error at any point, send that to the subscriber.
     
     绑定:
     
     1. 订阅值的原始信号。
     2. 任何时候原始信号发送一个值，使用绑定块转换它。
     3. 如果绑定块返回一个信号，则订阅该信号，并在接收到该信号时将其所有值传递给订阅方。
     4. 如果绑定块要求绑定终止，则完成_original_信号。
     5. 当_all_信号完成时，将完成发送到订阅服务器。
     
     如果任何信号在任何点发送错误，将其发送给订阅服务器。
    */
    
  
    return [[RACSignal createSignal:^(id<RACSubscriber> subscriber) {
        RACSignalBindBlock bindingBlock = block();
        // 首先根据传入的block生成一个RACSignalBindBlock(返回值是RACSignal对象)类型的bindingBlock
        
        __block volatile int32_t signalCount = 1;   // indicates self
        
        RACCompoundDisposable *compoundDisposable = [RACCompoundDisposable compoundDisposable];
        
        /*
         1.声明2个block，一个是completeSignal，一个是addSignal.
         2.completeSignal用于新的signal send completed
         3.addSignal用于sendNext、sendError、调用completeSignal。
         4.这里值得注意的是会在一开始声明一个int32_t类型的signalCount，在addSignalBlock中对signalCount执行了一下OSAtomicIncrement32Barrier原子操作，对signalCount进行+1，同时在completeSignalBlock里面进行减一操作，目的是防止bindSignal进行completed操作，而不是originSignal的sendCompleted操作而导致的completed操作。
        
        */
        void (^completeSignal)(RACDisposable *) = ^(RACDisposable *finishedDisposable) {
            if (OSAtomicDecrement32Barrier(&signalCount) == 0) {
                [subscriber sendCompleted];
                [compoundDisposable dispose];
            } else {
                [compoundDisposable removeDisposable:finishedDisposable];
            }
        };
        
        void (^addSignal)(RACSignal *) = ^(RACSignal *signal) {
            OSAtomicIncrement32Barrier(&signalCount);
            
            RACSerialDisposable *selfDisposable = [[RACSerialDisposable alloc] init];
            [compoundDisposable addDisposable:selfDisposable];
            
            RACDisposable *disposable = [signal subscribeNext:^(id x) {
                [subscriber sendNext:x];
            } error:^(NSError *error) {
                [compoundDisposable dispose];
                [subscriber sendError:error];
            } completed:^{
                @autoreleasepool {
                    completeSignal(selfDisposable);
                }
            }];
            
            selfDisposable.disposable = disposable;
        };
        
        /*
         订阅originSignal,一旦绑定的block转变成signal且不为空，且compoundDisposable没有被dispose，执行2中所说的addSignal，立即将值发送给订阅者subscriber，如果转变的signal为空或者要终止绑定，原始的信号就complete，当所有的信号都complete，发送completed信号给订阅者subscriber，如果中途信号出现了任何error，都会把这个错误发送给subscriber
        */
        @autoreleasepool {
            RACSerialDisposable *selfDisposable = [[RACSerialDisposable alloc] init];
            [compoundDisposable addDisposable:selfDisposable];
            
            RACDisposable *bindingDisposable = [self subscribeNext:^(id x) {
                // Manually check disposal to handle synchronous errors.
                if (compoundDisposable.disposed) return;
                
                BOOL stop = NO;
                id signal = bindingBlock(x, &stop);
                
                @autoreleasepool {
                    if (signal != nil) addSignal(signal);
                    if (signal == nil || stop) {
                        [selfDisposable dispose];
                        completeSignal(selfDisposable);
                    }
                }
            } error:^(NSError *error) {
                [compoundDisposable dispose];
                [subscriber sendError:error];
            } completed:^{
                @autoreleasepool {
                    completeSignal(selfDisposable);
                }
            }];
            
            selfDisposable.disposable = bindingDisposable;
        }
        
        return compoundDisposable;
    }] setNameWithFormat:@"[%@] -bind:", self.name];
    /*
     bind的实现，其实是对原有的Signal进行了2次封装： 当原来的Signal发送消息之后，RACSignalBindBlock拿到原有Signal发送的信息，然后进行block处理，返回封装之后的signal， 而且不难发现，每一次的消息发送都会被包成一个signal，新的signal再进行消息的发送。 绑定之后的signal不会影响原有signal的订阅操作，类似于category操作。
    */
}

- (RACSignal *)concat:(RACSignal *)signal {
    return [[RACSignal createSignal:^(id<RACSubscriber> subscriber) {
        RACCompoundDisposable *compoundDisposable = [[RACCompoundDisposable alloc] init];
        
        RACDisposable *sourceDisposable = [self subscribeNext:^(id x) {
            [subscriber sendNext:x];
        } error:^(NSError *error) {
            [subscriber sendError:error];
        } completed:^{
            RACDisposable *concattedDisposable = [signal subscribe:subscriber];
            [compoundDisposable addDisposable:concattedDisposable];
        }];
        
        [compoundDisposable addDisposable:sourceDisposable];
        return compoundDisposable;
    }] setNameWithFormat:@"[%@] -concat: %@", self.name, signal];
    
    /*
     调用concat之后的signal的didSubscribe, 会先订阅前一个signal，并正常的执行前一个signal的didSubscribe，当前一个signal sendCompleted的时候，就开始订阅后一个signal，然后开始执行后一个signal的didSubscribe, 在concat之前，前后的signal会首先将各自的didSubscribe copy起来，然后在concat之后，新的signal的didSubscribe 再把对应的block copy。
     
     值得注意的是：后一个signal是在前一个signal sendCompleted之后订阅的，那么如果前一个信号没有sendCompleted，后一个信号是不会被订阅的，因此concat是一个有序的signal组合，concat得到的新的signal能收到两个signal发送的消息值。
    */
}

- (RACSignal *)zipWith:(RACSignal *)signal {
    NSCParameterAssert(signal != nil);
    
    return [[RACSignal createSignal:^(id<RACSubscriber> subscriber) {
        __block BOOL selfCompleted = NO;
        NSMutableArray *selfValues = [NSMutableArray array];
        
        __block BOOL otherCompleted = NO;
        NSMutableArray *otherValues = [NSMutableArray array];
        
        void (^sendCompletedIfNecessary)(void) = ^{
            @synchronized (selfValues) {
                BOOL selfEmpty = (selfCompleted && selfValues.count == 0);
                BOOL otherEmpty = (otherCompleted && otherValues.count == 0);
                if (selfEmpty || otherEmpty) [subscriber sendCompleted];
            }
        };
        
        void (^sendNext)(void) = ^{
            @synchronized (selfValues) {
                if (selfValues.count == 0) return;
                if (otherValues.count == 0) return;
                
                RACTuple *tuple = RACTuplePack(selfValues[0], otherValues[0]);
                [selfValues removeObjectAtIndex:0];
                [otherValues removeObjectAtIndex:0];
                
                [subscriber sendNext:tuple];
                sendCompletedIfNecessary();
            }
        };
        
        RACDisposable *selfDisposable = [self subscribeNext:^(id x) {
            @synchronized (selfValues) {
                [selfValues addObject:x ?: RACTupleNil.tupleNil];
                sendNext();
            }
        } error:^(NSError *error) {
            [subscriber sendError:error];
        } completed:^{
            @synchronized (selfValues) {
                selfCompleted = YES;
                sendCompletedIfNecessary();
            }
        }];
        
        RACDisposable *otherDisposable = [signal subscribeNext:^(id x) {
            @synchronized (selfValues) {
                [otherValues addObject:x ?: RACTupleNil.tupleNil];
                sendNext();
            }
        } error:^(NSError *error) {
            [subscriber sendError:error];
        } completed:^{
            @synchronized (selfValues) {
                otherCompleted = YES;
                sendCompletedIfNecessary();
            }
        }];
        
        return [RACDisposable disposableWithBlock:^{
            [selfDisposable dispose];
            [otherDisposable dispose];
        }];
    }] setNameWithFormat:@"[%@] -zipWith: %@", self.name, signal];
    /*
     调用zipWith之后的signal的didSubscribe，两个signal会各自发送值，当第一个signal发送消息的时候，selfValues数组将值保存下来，并调用sendNext回调，sendNext里面会判断两个数组是否为空，有一个为空，则会return，在completed回调里面会将selfCompleted标志位置为Yes，并调用sendCompleted回调，在回调里面，同样也会判断标记位，但同时也会判断数组是否为空，因此这个判断条件的满足只有两个signal都发送消息一一配对发送出去才会走进，因为只有在sendNext回调里面，将两个数组的第一个元素取出来并打包成元祖RACTuple发送出去，并清空数组里面的第一个元素。
     
     因为两个signal每次发送消息的时候，对应的数组都会先将值保存下来，只有在另一个signal也发送消息时，才会打包成RACTuple发送出去，也就是说，如果其中一个signal发送了消息，但是另一个signal一直没有发送消息，那么第一个signal发送的消息永远不会被zipWith之后的signal发送，这个值就没有意义啦，需要一一配对。
    */
}

@end

@implementation RACSignal (Subscription)

- (RACDisposable *)subscribe:(id<RACSubscriber>)subscriber {
    NSCAssert(NO, @"This method must be overridden by subclasses");// (即子类RACDynamicSignal实现)
    return nil;
}

// 订阅信号
- (RACDisposable *)subscribeNext:(void (^)(id x))nextBlock {
    NSCParameterAssert(nextBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:nextBlock error:NULL completed:NULL];
    return [self subscribe:o];
    /*
     1.此方法会创建一个订阅者对象 subscriber；(挪步至RACSubscriber)
     2.并传入一个名为 nextBlock，参数为 id 类型， 返回值为 RACDisposable 对象的 block回调，保存到subscriber 对象中；
     3.然后调用 signal 对象的订阅方法 subscribe: ，参数为前面的 subscriber 对象，返回值为 RACDisposable 对象，此方法中会调用 signal 对象中保存的 didSubscribe 回调；
     4.订阅(挪步至子类RACDynamicSignal)后信号变为热信号
     */
}

- (RACDisposable *)subscribeNext:(void (^)(id x))nextBlock completed:(void (^)(void))completedBlock {
    NSCParameterAssert(nextBlock != NULL);
    NSCParameterAssert(completedBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:nextBlock error:NULL completed:completedBlock];
    return [self subscribe:o];
}

- (RACDisposable *)subscribeNext:(void (^)(id x))nextBlock error:(void (^)(NSError *error))errorBlock completed:(void (^)(void))completedBlock {
    NSCParameterAssert(nextBlock != NULL);
    NSCParameterAssert(errorBlock != NULL);
    NSCParameterAssert(completedBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:nextBlock error:errorBlock completed:completedBlock];
    return [self subscribe:o];
}

- (RACDisposable *)subscribeError:(void (^)(NSError *error))errorBlock {
    NSCParameterAssert(errorBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:NULL error:errorBlock completed:NULL];
    return [self subscribe:o];
}

- (RACDisposable *)subscribeCompleted:(void (^)(void))completedBlock {
    NSCParameterAssert(completedBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:NULL error:NULL completed:completedBlock];
    return [self subscribe:o];
}

- (RACDisposable *)subscribeNext:(void (^)(id x))nextBlock error:(void (^)(NSError *error))errorBlock {
    NSCParameterAssert(nextBlock != NULL);
    NSCParameterAssert(errorBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:nextBlock error:errorBlock completed:NULL];
    return [self subscribe:o];
}

- (RACDisposable *)subscribeError:(void (^)(NSError *))errorBlock completed:(void (^)(void))completedBlock {
    NSCParameterAssert(completedBlock != NULL);
    NSCParameterAssert(errorBlock != NULL);
    
    RACSubscriber *o = [RACSubscriber subscriberWithNext:NULL error:errorBlock completed:completedBlock];
    return [self subscribe:o];
}

@end

@implementation RACSignal (Debugging)

- (RACSignal *)logAll {
    return [[[self logNext] logError] logCompleted];
}

- (RACSignal *)logNext {
    return [[self doNext:^(id x) {
        NSLog(@"%@ next: %@", self, x);
    }] setNameWithFormat:@"%@", self.name];
}

- (RACSignal *)logError {
    return [[self doError:^(NSError *error) {
        NSLog(@"%@ error: %@", self, error);
    }] setNameWithFormat:@"%@", self.name];
}

- (RACSignal *)logCompleted {
    return [[self doCompleted:^{
        NSLog(@"%@ completed", self);
    }] setNameWithFormat:@"%@", self.name];
}

@end

@implementation RACSignal (Testing)

static const NSTimeInterval RACSignalAsynchronousWaitTimeout = 10;

- (id)asynchronousFirstOrDefault:(id)defaultValue success:(BOOL *)success error:(NSError **)error {
    return [self asynchronousFirstOrDefault:defaultValue success:success error:error timeout:RACSignalAsynchronousWaitTimeout];
}

- (id)asynchronousFirstOrDefault:(id)defaultValue success:(BOOL *)success error:(NSError **)error timeout:(NSTimeInterval)timeout {
    NSCAssert([NSThread isMainThread], @"%s should only be used from the main thread", __func__);
    
    __block id result = defaultValue;
    __block BOOL done = NO;
    
    // Ensures that we don't pass values across thread boundaries by reference.
    __block NSError *localError;
    __block BOOL localSuccess = YES;
    
    [[[[self
        take:1]
       timeout:timeout onScheduler:[RACScheduler scheduler]]
      deliverOn:RACScheduler.mainThreadScheduler]
     subscribeNext:^(id x) {
        result = x;
        done = YES;
    } error:^(NSError *e) {
        if (!done) {
            localSuccess = NO;
            localError = e;
            done = YES;
        }
    } completed:^{
        done = YES;
    }];
    
    do {
        [NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    } while (!done);
    
    if (success != NULL) *success = localSuccess;
    if (error != NULL) *error = localError;
    
    return result;
}

- (BOOL)asynchronouslyWaitUntilCompleted:(NSError **)error timeout:(NSTimeInterval)timeout {
    BOOL success = NO;
    [[self ignoreValues] asynchronousFirstOrDefault:nil success:&success error:error timeout:timeout];
    return success;
}

- (BOOL)asynchronouslyWaitUntilCompleted:(NSError **)error {
    return [self asynchronouslyWaitUntilCompleted:error timeout:RACSignalAsynchronousWaitTimeout];
}

@end
