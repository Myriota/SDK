# Message Unpacker

A CloudFormation template that creates a [destination](https://developer.myriota.com/code-examples-lambda) for use with the Myriota Cloud. The destination is implemented as a Lambda function which unpacks and saves messages to DynamoDB.

## Deploying template via AWS CLI

To deploy the stack via the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html), `cd` into the directory containing the file and run the below command:

```bash
aws cloudformation create-stack --stack-name MessageUnpacker --template-body file://stack.yaml --capabilities CAPABILITY_IAM
```

### Updating the stack

If you make changes to the template, you can deploy the changes by running the above command with `update-stack` instead of `create-stack`. More information on updating a stack can be found [here.](https://docs.aws.amazon.com/cli/latest/reference/cloudformation/update-stack.html)

## Template Outputs

The deployed template has two output values, the Lambda function's ARN and the ARN for the Role to be assumed by Myriota to invoke the function.
These two [ARNs](https://docs.aws.amazon.com/general/latest/gr/aws-arns-and-namespaces.html) are used to configure your module's destination in the Device Manager. Please follow the `Create a Destination` instructions [found here.](https://developer.myriota.com/code-examples-lambda)
